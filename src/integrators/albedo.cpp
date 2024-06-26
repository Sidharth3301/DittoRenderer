#include <lightwave.hpp>

namespace lightwave
{

   
    class AlbedoIntegrator : public SamplingIntegrator
    {
        /// @brief Whether to show the grid, or only output the ray's direction as color.
        bool m_showGrid;
        /// @brief The color of the grid, if the grid is shown.
        Color m_gridColor;
        /// @brief The frequency of the grid spacing, if the grid is shown.
        float m_gridFrequency;

        bool remap;

    public:
        AlbedoIntegrator(const Properties &properties)
            : SamplingIntegrator(properties)
        {
            // to parse properties from the scene description, use properties.get(name, default_value)
            // you can also omit the default value if you want to require the user to specify a value

            m_showGrid = properties.get<bool>("grid", true);
            m_gridColor = properties.get<Color>("gridColor", Color::black());
            m_gridFrequency = properties.get<float>("gridFrequency", 10);
            // remap = properties.get<bool>("remap", true);
        }

        /**
         * @brief The job of an integrator is to return a color for a ray produced by the camera model.
         * This will be run for each pixel of the image, potentially with multiple samples for each pixel.
         */
        Color Li(const Ray &ray, Sampler &rng) override
        {
            Intersection its = m_scene->intersect(ray, rng);
            Color albedo;
            if (its.t != Infinity)
            {   
                if (its.instance->bsdf()){
                albedo = its.instance->bsdf()->getAlbedo(its.uv, its.wo);
                }
                else
                albedo = its.instance->emission()->evaluate(its.uv, its.wo).value;
            }
            else
                albedo = Color(0., 0., 0.);

            
            return albedo;
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
REGISTER_INTEGRATOR(AlbedoIntegrator, "albedo")
