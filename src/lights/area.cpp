#include <lightwave.hpp>

namespace lightwave
{

    class AreaLight final : public Light
    {
        Point pLight;
        Color Power;
        ref<Instance> shape;

    public:
        AreaLight(const Properties &properties)
        {
            Power = properties.get<Color>("power");
            shape = properties.get<Instance>("instance");
        }

        DirectLightSample sampleDirect(const Point &origin,
                                       Sampler &rng) const override
        {
            DirectLightSample Li;
            AreaSample areasample = shape->sampleArea(rng);

            Vector dir = areasample.position - origin;
            Li.wi = dir.normalized();
            
            Li.weight = shape->emission()->evaluate(areasample.uv, Li.wi).value/ (areasample.pdf* dir.lengthSquared());
            Li.distance = dir.length();
            return Li;
        }

        bool canBeIntersected() const override { return shape->isVisible(); }

        std::string toString() const override
        {
            return tfm::format("Arealight[\n"
                               "]");
        }
    };

} // namespace lightwave

REGISTER_LIGHT(AreaLight, "area")
