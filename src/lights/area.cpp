#include <lightwave.hpp>

namespace lightwave
{

    class AreaLight final : public Light
    {
        ref<Instance> m_instance;

    public:
        AreaLight(const Properties &properties)
        {
            m_instance = properties.getChild<Instance>();
        }

        DirectLightSample sampleDirect(const Point &origin,
                                       Sampler &rng) const override
        {
            DirectLightSample Li;
            AreaSample areasample;
            if (m_instance)
            { areasample = m_instance->sampleArea(rng);}
            else
            {logger(EInfo, "Not found the shape");}

            Vector dir = areasample.position - origin;
            Li.wi = dir.normalized();
            auto costheta = areasample.frame.normal.dot(Li.wi);
            auto intensity = m_instance->emission()->evaluate(areasample.uv, Li.wi).value;
            Li.weight = intensity*costheta/ (areasample.pdf*dir.lengthSquared());
            Li.distance = dir.length();
            return Li;
        }

        bool canBeIntersected() const override { return false; }

        std::string toString() const override
        {
            return tfm::format("Arealight[\n"
                               "]");
        }
    };

} // namespace lightwave

REGISTER_LIGHT(AreaLight, "area")
