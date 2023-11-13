#include <lightwave.hpp>

namespace lightwave
{

    /**
     * @brief Visualizes the rays generated by a camera by intersecting them with a planar grid, and encoding the
     * ray's direction as color. The grid is an infinite plane at z=+1 with a grid frequency specified by the user.
     *
     * @example This integrator can be used as following:
     * @code
     *   <integrator type="camera">
     *     <boolean name="grid" value="true" />
     *     <color name="gridColor" value="0.5, 0.3, 0.2" />
     *     <float name="gridFrequency" value="5" />
     *   </integrator>
     * @endcode
     * or equivalently, but more succintly:
     * @code
     *   <integrator type="camera" grid="true" gridColor="0.5,0.3,0.2" gridFrequency="5" />
     * @endcode
     */
    class NormalIntegrator : public SamplingIntegrator
    {
        /// @brief Whether to show the grid, or only output the ray's direction as color.
        bool m_showGrid;
        /// @brief The color of the grid, if the grid is shown.
        Color m_gridColor;
        /// @brief The frequency of the grid spacing, if the grid is shown.
        float m_gridFrequency;

        bool remap;

    public:
        NormalIntegrator(const Properties &properties)
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
            Intersection its = m_scene->intersect(ray, rng);
            Vector normal;
            if (its.t != Infinity)
            {
                normal = its.frame.normal;
            }
            else
                normal = Vector(0., 0., 0.);

            if (remap){
                normal = (normal+Vector(1))/2.f;
            }
            return Color(normal);
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
REGISTER_INTEGRATOR(NormalIntegrator, "normals")
