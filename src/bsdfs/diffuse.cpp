#include <lightwave.hpp>

namespace lightwave
{

    class Diffuse : public Bsdf
    {
        ref<Texture> m_albedo;

    public:
        Diffuse(const Properties &properties)
        {
            m_albedo = properties.get<Texture>("albedo");
        }

        BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                          const Vector &wi) const override
        {
            if (Frame::cosTheta(wi) * Frame::cosTheta(wo) <= 0)
            {
                return BsdfEval::invalid();
            }
            return BsdfEval{.value = (m_albedo->evaluate(uv) * Frame::cosTheta(wi)) *InvPi};
        }
        Color getAlbedo(const Point2 &uv, const Vector &wo) const override
        {
            return m_albedo->evaluate(uv);
        }

        BsdfSample sample(const Point2 &uv, const Vector &wo,
                          Sampler &rng) const override
        {
            BsdfSample b;
            b.wi = squareToCosineHemisphere(rng.next2D()).normalized();
            b.weight = m_albedo->evaluate(uv);
            return b;
        }

        std::string toString() const override
        {
            return tfm::format("Diffuse[\n"
                               "  albedo = %s\n"
                               "]",
                               indent(m_albedo));
        }
    };

} // namespace lightwave

REGISTER_BSDF(Diffuse, "diffuse")
