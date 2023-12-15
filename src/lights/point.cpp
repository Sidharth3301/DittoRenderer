#include <lightwave.hpp>

namespace lightwave
{

    class PointLight final : public Light
    {
        Point pLight;
        Color Power;

    public:
        PointLight(const Properties &properties)
        {
            pLight = properties.get<Point>("position");
            Power = properties.get<Color>("power");
        }

        DirectLightSample sampleDirect(const Point &origin,
                                       Sampler &rng) const override
        {
            DirectLightSample Li;
            Vector dir = pLight - origin;
            Li.wi = dir.normalized();
            auto I = Power / (4*Pi); //intensity = power/solid angle  
            Li.weight = I / dir.lengthSquared();
            Li.distance = dir.length();
            return Li;
        }

        bool canBeIntersected() const override { return false; }

        std::string toString() const override
        {
            return tfm::format("PointLight[\n"
                               "]");
        }
    };

} // namespace lightwave

REGISTER_LIGHT(PointLight, "point")
