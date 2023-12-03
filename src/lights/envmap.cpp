#include <lightwave.hpp>

namespace lightwave
{

    class EnvironmentMap final : public BackgroundLight
    {
        /// @brief The texture to use as background
        ref<Texture> m_texture;

        /// @brief An optional transform from local-to-world space
        ref<Transform> m_transform;

    public:
        EnvironmentMap(const Properties &properties)
        {
            m_texture = properties.getChild<Texture>();
            m_transform = properties.getOptionalChild<Transform>();
        }

        BackgroundLightEval evaluate(const Vector &direction) const override
        {
            Vector2 warped = Vector2(0, 0);
            // hints:
            // * if (m_transform) { transform direction vector from world to local
            // coordinates }
            // * find the corresponding pixel coordinate for the given local
            // direction
            //
            auto local_dir = direction;
            if (m_transform)
            {
                local_dir = m_transform->inverse(direction).normalized();
            }
            auto x = local_dir.x();
            auto y = local_dir.y();
            auto z = local_dir.z();
            float phi, theta;
            theta = atan2(sqrt(std::pow(x, 2) + std::pow(z, 2)), y);
            phi = atan2(z, x);
            phi -= Pi;

            warped.y() = theta / Pi;
            warped.x() = 0.5- ((phi+Pi) / (2 * Pi));
            // warped.y() = 1- warped.y();
            return {
                .value = m_texture->evaluate(warped),
            };
        }

        DirectLightSample sampleDirect(const Point &origin,
                                       Sampler &rng) const override
        {
            Vector direction = squareToUniformSphere(rng.next2D());
            auto E = evaluate(direction);

            // implement better importance sampling here, if you ever need it
            // (useful for environment maps with bright tiny light sources, like the
            // sun for example)

            return {
                .wi = direction,
                .weight = E.value / Inv4Pi,
                .distance = Infinity,
            };
        }

        std::string toString() const override
        {
            return tfm::format("EnvironmentMap[\n"
                               "  texture = %s,\n"
                               "  transform = %s\n"
                               "]",
                               indent(m_texture), indent(m_transform));
        }
    };

} // namespace lightwave

REGISTER_LIGHT(EnvironmentMap, "envmap")
