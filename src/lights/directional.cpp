#include <lightwave.hpp>

namespace lightwave
{

    class DirectionalLight final : public Light
    {
        Vector direction;
        Color intensity;

    public:
        DirectionalLight(const Properties &properties)
        {
            Vector dir = properties.get<Vector>("direction", Vector(0, -1, 0)); // Default direction
            intensity = properties.get<Color>("intensity", Color(1.0f)); // Default intensity

            // Normalize the direction vector
            direction = dir.normalized();
        }

        DirectLightSample sampleDirect(const Point &origin,
                                       Sampler &rng) const override
        {
            DirectLightSample dls;
            dls.wi = direction;
            dls.distance = Infinity;
            dls.weight = intensity;
            return dls;
        }

        bool canBeIntersected() const override { return false; }

        std::string toString() const override
        {
            return tfm::format("DirectionalLight[\n"
                               "  dir = %s\n"
                               "  intensity  = %s\n"
                               "]",
                               indent(direction),
                               indent(intensity));
        }
    };

} // namespace lightwave

REGISTER_LIGHT(DirectionalLight, "directional")
