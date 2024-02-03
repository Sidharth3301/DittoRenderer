#include <lightwave.hpp>

namespace lightwave
{

    class AreaLight final : public Light
    {
        ref<Instance> shape;

    public:
        AreaLight(const Properties &properties)
        {
            shape = properties.getOptionalChild<Instance>();
        }

        DirectLightSample sampleDirect(const Point &origin,
                                       Sampler &rng) const override
        {
            DirectLightSample Li;
            AreaSample areasample = shape->sampleArea(rng);

            Vector dir = areasample.position - origin;
            Li.wi = dir.normalized();
            Li.weight = shape->emission()->evaluate(areasample.uv, Li.wi).value/ (areasample.pdf);
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
