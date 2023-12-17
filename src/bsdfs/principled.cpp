#include <lightwave.hpp>

#include "fresnel.hpp"
#include "microfacet.hpp"

namespace lightwave
{

    struct DiffuseLobe
    {
        Color color;

        BsdfEval evaluate(const Vector &wo, const Vector &wi) const
        {
            return BsdfEval{.value = (color) / Pi};

            // hints:
            // * copy your diffuse bsdf evaluate here
            // * you do not need to query a texture, the albedo is given by `color`
        }

        BsdfSample sample(const Vector &wo, Sampler &rng) const
        {
            BsdfSample b;
            b.wi = squareToCosineHemisphere(rng.next2D()).normalized();
            b.weight = color;
            return b;

            // hints:
            // * copy your diffuse bsdf evaluate here
            // * you do not need to query a texture, the albedo is given by `color`
        }
    };

    struct MetallicLobe
    {
        float alpha;
        Color color;

        BsdfEval evaluate(const Vector &wo, const Vector &wi) const
        {
            //    const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));

            auto mfacet_normal = (wi + wo).normalized();
            auto reflectance = color;
            auto D = microfacet::evaluateGGX(alpha, mfacet_normal);
            auto Gwi = microfacet::smithG1(alpha, mfacet_normal, wi);
            auto Gwo = microfacet::smithG1(alpha, mfacet_normal, wo);
            auto fr = reflectance * D * Gwi * Gwo / (4 * fabs(Frame::cosTheta(wi)) * fabs(Frame::cosTheta(wo)));
            return BsdfEval{.value = fr};

            // hints:
            // * copy your roughconductor bsdf evaluate here
            // * you do not need to query textures
            //   * the reflectance is given by `color'
            //   * the variable `alpha' is already provided for you
        }

        BsdfSample sample(const Vector &wo, Sampler &rng) const
        {
            BsdfSample samp;
            auto mf_normal = microfacet::sampleGGXVNDF(alpha, wo, rng.next2D());
            if (microfacet::pdfGGXVNDF(alpha, mf_normal, wo) < 0)
            {
                return BsdfSample::invalid();
            }
            samp.wi = reflect(wo, mf_normal);
            auto sign = Frame::cosTheta(samp.wi) > 0 ? 1 : -1;
            samp.weight = color * microfacet::smithG1(alpha, mf_normal, samp.wi) * sign;
            return samp;

            // hints:
            // * copy your roughconductor bsdf sample here
            // * you do not need to query textures
            //   * the reflectance is given by `color'
            //   * the variable `alpha' is already provided for you
        }
    };

    class Principled : public Bsdf
    {
        ref<Texture> m_baseColor;
        ref<Texture> m_roughness;
        ref<Texture> m_metallic;
        ref<Texture> m_specular;

        struct Combination
        {
            float diffuseSelectionProb;
            DiffuseLobe diffuse;
            MetallicLobe metallic;
        };

        Combination combine(const Point2 &uv, const Vector &wo) const
        {
            const auto baseColor = m_baseColor->evaluate(uv);
            const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));
            const auto specular = m_specular->scalar(uv);
            const auto metallic = m_metallic->scalar(uv);
            const auto F =
                specular * schlick((1 - metallic) * 0.08f, Frame::cosTheta(wo));

            const DiffuseLobe diffuseLobe = {
                .color = (1 - F) * (1 - metallic) * baseColor,
            };
            const MetallicLobe metallicLobe = {
                .alpha = alpha,
                .color = F * Color(1) + (1 - F) * metallic * baseColor,
            };

        const auto diffuseAlbedo = diffuseLobe.color.mean();
        const auto totalAlbedo =
            diffuseLobe.color.mean() + metallicLobe.color.mean();
        const auto diffuseAlbedo = diffuseLobe.color.mean();
        const auto totalAlbedo =
            diffuseLobe.color.mean() + metallicLobe.color.mean();
        return {
            .diffuseSelectionProb =
                totalAlbedo > 0 ? diffuseAlbedo / totalAlbedo : 1.0f,
                totalAlbedo > 0 ? diffuseAlbedo / totalAlbedo : 1.0f,
            .diffuse  = diffuseLobe,
            .metallic = metallicLobe,
        };
    }

    public:
        Principled(const Properties &properties)
        {
            m_baseColor = properties.get<Texture>("baseColor");
            m_roughness = properties.get<Texture>("roughness");
            m_metallic = properties.get<Texture>("metallic");
            m_specular = properties.get<Texture>("specular");
        }

        BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                          const Vector &wi) const override
        {
            const auto combination = combine(uv, wo);
            return BsdfEval{.value = Frame::cosTheta(wi) * (combination.diffuse.evaluate(wo, wi).value + combination.metallic.evaluate(wo, wi).value)};

            // hint: evaluate `combination.diffuse` and `combination.metallic` and
            // combine their results
        }

        BsdfSample sample(const Point2 &uv, const Vector &wo,
                          Sampler &rng) const override
        {
            const auto combination = combine(uv, wo);
            BsdfSample bsdfSample;
            auto prob = combination.diffuseSelectionProb;
            if (rng.next() < prob)
            {

                auto diffuse_sample = combination.diffuse.sample(wo, rng);
                bsdfSample.wi = diffuse_sample.wi;
                bsdfSample.weight = diffuse_sample.weight / prob;
                if (std::isnan(prob))
                {
                    return BsdfSample::invalid();
                }
                // assert_finite(bsdfSample.weight, {});
            }
            else
            {

                auto metallic_sample = combination.metallic.sample(wo, rng);
                bsdfSample.wi = metallic_sample.wi;
                float metallic_prob = 1 - prob;
                if (metallic_sample.isInvalid())
                {
                    return BsdfSample::invalid();
                }
                bsdfSample.weight = metallic_sample.weight / metallic_prob;
                // assert_finite(bsdfSample.weight, {});
                // logger(EDebug, "metallic prob %f", metallic_prob);
            }

            return bsdfSample;

            // hint: sample either `combination.diffuse` (probability
            // `combination.diffuseSelectionProb`) or `combination.metallic`
        }

        std::string toString() const override
        {
            return tfm::format("Principled[\n"
                               "  baseColor = %s,\n"
                               "  roughness = %s,\n"
                               "  metallic  = %s,\n"
                               "  specular  = %s,\n"
                               "]",
                               indent(m_baseColor), indent(m_roughness),
                               indent(m_metallic), indent(m_specular));
        }
    };

} // namespace lightwave

REGISTER_BSDF(Principled, "principled")
