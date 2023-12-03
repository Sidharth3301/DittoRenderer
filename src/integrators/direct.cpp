
#include <lightwave.hpp>

namespace lightwave
{

    class DirectIntegrator : public SamplingIntegrator
    {
        /// @brief Whether to show the grid, or only output the ray's direction as color.
        bool m_showGrid;
        /// @brief The color of the grid, if the grid is shown.
        Color m_gridColor;
        /// @brief The frequency of the grid spacing, if the grid is shown.
        float m_gridFrequency;

        bool remap;

    public:
        DirectIntegrator(const Properties &properties)
            : SamplingIntegrator(properties)
        {
            // to parse properties from the scene description, use properties.get(name, default_value)
            // you can also omit the default value if you want to require the user to specify a value

            m_showGrid = properties.get<bool>("grid", true);
            m_gridColor = properties.get<Color>("gridColor", Color::black());
            m_gridFrequency = properties.get<float>("gridFrequency", 10);
            remap = properties.get<bool>("remap", true);
        }

        /**
         * @brief The job of an integrator is to return a color for a ray produced by the camera model.
         * This will be run for each pixel of the image, potentially with multiple samples for each pixel.
         */
        Color Li(const Ray &ray, Sampler &rng) override
        {
            Color color = Color(0.0f);
            Color weight = Color(1.0f);

            Intersection first_its = m_scene->intersect(ray, rng);
            if (!first_its)
            {
                return m_scene->evaluateBackground(ray.direction).value;
            }
            else
            {
                BsdfSample b = first_its.sampleBsdf(rng);
                color += first_its.evaluateEmission();
                if (b.isInvalid())
                {
                    return color;
                }
                weight *= b.weight;
                Ray second_ray{first_its.position, b.wi};
                Intersection second_its = m_scene->intersect(second_ray, rng);
                if (!second_its)
                {
                    color += m_scene->evaluateBackground(second_ray.direction).value;
                }
                else
                {
                    color += second_its.evaluateEmission();
                    if (b.isInvalid())
                    {
                        return color;
                    }
                }
            }
            return color * weight;
        }

        /// @brief An optional textual representation of this class, which can be useful for debugging.
        std::string toString() const override
        {
            return tfm::format(
                "normalIntegrator[\n"
                "  sampler = %s,\n"
                "  image = %s,\n"
                "]",
                indent(m_sampler),
                indent(m_image));
        }
    };

}

// this informs lightwave to use our class CameraIntegrator whenever a <integrator type="camera" /> is found in a scene file
REGISTER_INTEGRATOR(DirectIntegrator, "direct")
