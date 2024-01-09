#include <lightwave.hpp>
#include <string>
using namespace std;

namespace lightwave {

/**
 * @brief A perspective camera with a given field of view angle and transform.
 * 
 * In local coordinates (before applying m_transform), the camera looks in positive z direction [0,0,1].
 * Pixels on the left side of the image ( @code normalized.x < 0 @endcode ) are directed in negative x
 * direction ( @code ray.direction.x < 0 ), and pixels at the bottom of the image ( @code normalized.y < 0 @endcode )
 * are directed in negative y direction ( @code ray.direction.y < 0 ).
 */
class ThinLens : public Camera {
    private:
    Vector origin;
    float scale_x;
    float scale_y;
    string fovAxis;
    float m_apertureRadius;
    float focal_length;

public:
    ThinLens(const Properties &properties)
    : Camera(properties) {
        // NOT_IMPLEMENTED
        origin = Vector(0.f, 0.f, 0.f);
        float fov = properties.get<float>("fov");
        fovAxis = properties.get<string>("fovAxis");
        m_apertureRadius = properties.get<float>("apartureRadius");
        focal_length = properties.get<float>("f");

        if (fovAxis == "x")
        {
            float aspect_ratio = (float)m_resolution.x()/(float)m_resolution.y();

            scale_x = tan(Deg2Rad*(fov/2.f));
            scale_y = scale_x/aspect_ratio;

        }
        if(fovAxis== "y"){
            float aspect_ratio = (float)m_resolution.y()/(float)m_resolution.x();
            scale_y = tan(Deg2Rad*(fov/2.f));
            scale_x = scale_y/aspect_ratio;
        }
          /* World-space aperture radius */

        if (m_apertureRadius == 0) {
            m_apertureRadius = Epsilon;
        }
        // hints:
        // * precompute any expensive operations here (most importantly trigonometric functions)
        // * use m_resolution to find the aspect ratio of the image
    }

    CameraSample sample(const Point2 &normalized, Sampler &rng) const override 
        // NOT_IMPLEMENTED
       {
            
        //   // Sample point on lens using warp function
        //     Point2 lens_point = squareToUniformDiskConcentric(rng.next2D()) * m_apertureRadius;

        //     // Compute point on the focal plane
        //     Vector direction(normalized.x() * scale_x, normalized.y() * scale_y, 1.f);
        //     direction = direction.normalized();
        //     Vector focalPoint = origin + direction * focal_length;

        //     // Adjust ray for depth of field
        //     Ray ray(origin + Vector(lens_point.x(), lens_point.y(), 0.f), focalPoint - origin);


            Point2 lensSample = squareToUniformDiskConcentric(rng.next2D()) * m_apertureRadius;

            Vector lensPoint = Vector(lensSample.x(), lensSample.y(), 0.0f);

            Vector direction = Vector(normalized.x() * scale_x, normalized.y() * scale_y, 1.0f);
            direction = direction.normalized();

            Vector focalPoint = origin + direction * focal_length;

            Ray ray = Ray(origin + lensPoint, (focalPoint - (origin + lensPoint)).normalized());



           // Ray ray = Ray(origin, Vector(normalized.x()*scale_x, normalized.y()*scale_y, 1.f));
            
            return CameraSample{
                .ray = m_transform->apply(ray.normalized()),
                .weight = Color(1.0f)};
        
        }

        // hints:
        // * use m_transform to transform the local camera coordinate system into the world coordinate system
    

    std::string toString() const override {
        return tfm::format(
            "ThinLens[\n"
            "  width = %d,\n"
            "  height = %d,\n"
            "  transform = %s,\n"
            "]",
            m_resolution.x(),
            m_resolution.y(),
            indent(m_transform)
        );
    }
};

}

REGISTER_CAMERA(ThinLens, "thinlens")
