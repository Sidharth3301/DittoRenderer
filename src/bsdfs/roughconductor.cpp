#include "fresnel.hpp"
#include "microfacet.hpp"
#include <lightwave.hpp>

namespace lightwave
{

    class RoughConductor : public Bsdf
    {
        ref<Texture> m_reflectance;
        ref<Texture> m_roughness;

    public:
        RoughConductor(const Properties &properties)
        {
            m_reflectance = properties.get<Texture>("reflectance");
            m_roughness = properties.get<Texture>("roughness");
        }

        BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                          const Vector &wi) const override
        {
            // Using the squared roughness parameter results in a more gradual
            // transition from specular to rough. For numerical stability, we avoid
            // extremely specular distributions (alpha values below 10^-3)
            const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));

            auto mfacet_normal = (wi + wo).normalized();
            auto reflectance = m_reflectance->evaluate(uv);
            auto D = microfacet::evaluateGGX(alpha, mfacet_normal);
            auto Gwi = microfacet::smithG1(alpha, mfacet_normal, wi);
            auto Gwo = microfacet::smithG1(alpha, mfacet_normal, wo);
            auto fr = reflectance * D * Gwi * Gwo / (4 * fabs(Frame::cosTheta(wi)) * fabs(Frame::cosTheta(wo)));
            return BsdfEval{.value = fr};
            // hints:
            // * the microfacet normal can be computed from `wi' and `wo'
        }

        BsdfSample sample(const Point2 &uv, const Vector &wo,
                          Sampler &rng) const override
        {
            const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));
            BsdfSample samp;
            auto mf_normal = microfacet::sampleGGXVNDF(alpha, wo, rng.next2D());
            if (microfacet::pdfGGXVNDF(alpha, mf_normal, wo) < 0)
            {
                return BsdfSample::invalid();
            }
            samp.wi = reflect(wo, mf_normal);
            auto sign = Frame::cosTheta(samp.wi) > 0 ? 1 : -1;
            samp.weight = m_reflectance->evaluate(uv) * microfacet::smithG1(alpha, mf_normal, samp.wi) * sign;

            return samp;
            // hints:
            // * do not forget to cancel out as many terms from your equations as possible!
            //   (the resulting sample weight is only a product of two factors)
        }

        std::string toString() const override
        {
            return tfm::format("RoughConductor[\n"
                               "  reflectance = %s,\n"
                               "  roughness = %s\n"
                               "]",
                               indent(m_reflectance), indent(m_roughness));
        }
    };

} // namespace lightwave

REGISTER_BSDF(RoughConductor, "roughconductor")
