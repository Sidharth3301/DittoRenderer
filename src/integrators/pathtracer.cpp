
#include <lightwave.hpp>

namespace lightwave
{

    class Pathtracer : public SamplingIntegrator
    {
        /// @brief Whether to show the grid, or only output the ray's direction as color.
        bool m_showGrid;
        /// @brief The color of the grid, if the grid is shown.
        Color m_gridColor;
        /// @brief The frequency of the grid spacing, if the grid is shown.
        float m_gridFrequency;

        int depth;

    public:
        Pathtracer(const Properties &properties)
            : SamplingIntegrator(properties)
        {
            // to parse properties from the scene description, use properties.get(name, default_value)
            // you can also omit the default value if you want to require the user to specify a value

            m_showGrid = properties.get<bool>("grid", true);
            m_gridColor = properties.get<Color>("gridColor", Color::black());
            m_gridFrequency = properties.get<float>("gridFrequency", 10);
            depth = properties.get<int>("depth", 2);
        }

        /**
         * @brief The job of an integrator is to return a color for a ray produced by the camera model.
         * This will be run for each pixel of the image, potentially with multiple samples for each pixel.
         */
        Color Li(const Ray &ray, Sampler &rng) override
        {
            Color color{0.0f};
            Color weight{1.0f};
            Ray curr_ray = ray;

            while (true)
            {
                Intersection its = m_scene->intersect(curr_ray, rng);
                if (!its)
                {
                    color += m_scene->evaluateBackground(curr_ray.direction).value * weight;
                    break;
                }
                else
                {
                    color += its.evaluateEmission()*weight;
                    if (m_scene->hasLights())
                    {
                        LightSample ls = m_scene->sampleLight(rng);
                        if (!ls.light->canBeIntersected())
                        {
                            DirectLightSample dls = ls.light->sampleDirect(its.position, rng);

                            Vector toLight = dls.wi; // Directional light's direction
                            bool isIntersecting = m_scene->intersect(Ray(its.position, toLight), dls.distance, rng);

                            if (!isIntersecting)
                            { // Check if light direction is visible
                                Color bsdfVal = its.evaluateBsdf(toLight).value;
                                color += (bsdfVal * dls.weight *weight / ls.probability);
                            }
                        }
                    }
                    if (curr_ray.depth>=depth-1){
                        return color;
                    }
                    BsdfSample b = its.sampleBsdf(rng);
                    if (b.isInvalid())
                    {
                        return color;
                    }
                    weight *= b.weight;
                    curr_ray.origin = its.position;
                    curr_ray.direction = b.wi;
                    curr_ray.depth +=1;
                }
            }
            return color;
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
REGISTER_INTEGRATOR(Pathtracer, "pathtracer")
