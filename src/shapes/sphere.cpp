#include <lightwave.hpp>
#include <ostream>
namespace lightwave
{
    class Sphere : public Shape
    {
        inline void populate(SurfaceEvent &surf, const Point &position) const
        {
            surf.position = position;

            float theta = atan2f(position.x(), position.z());
            float phi = acosf(position.y());
            surf.uv.x() = (theta + Pi) / (2 * Pi);
            surf.uv.y() = (phi) / Pi;
            Vector normal = (position - Point(0., 0., 0.)).normalized();
            surf.frame = Frame(normal);

            // since we sample the area uniformly, the pdf is given by 1/surfaceArea
            surf.pdf = 0;
        }

    public:
        Sphere(const Properties &properties)
        {
        }
        bool intersect(const Ray &ray, Intersection &its, Sampler &rng) const override
        {
            float discriminant;
            Vector o = Vector(ray.origin);
            Vector d = ray.direction;
            auto a = d.dot(d);
            auto b = 2 * o.dot(d);
            auto c = o.dot(o) - 1.f;

            discriminant = b * b - 4 * a * c;
            // logger(EDebug, "hitpoint rn at %.2f", its.t);
            float t0, t1;
            if (discriminant < 0) // no real solution if Disc<0
                return false;
            else
            {
                float q = (b > 0) ? -0.5 * (b + sqrt(discriminant)) : -0.5 * (b - sqrt(discriminant));
                t0 = q / a;
                t1 = c / q;
            }

            if (t0 > t1)
            {
                float tmp = t0;
                t0 = t1;
                t1 = tmp;
            }
            if (t0 < 1e-4f)
            {
                t0 = t1; // if t0 is negative, let's use t1 instead
                if (t0 < 0 || t0 < 1e-4f)
                    return false; // both t0 and t1 are negative
            }

            if (t0 > its.t)
                return false;

            its.t = t0;
            Point position = ray(its.t);
            populate(its, position);
            return true;
        }

        Bounds
        getBoundingBox() const override
        {
            Point a0 = Point(-1, -1, -1);
            Point a1 = Point(1, 1, 1);
            return Bounds(a0, a1);
        }
        Point getCentroid() const override
        {
            return Point(0, 0, 0);
        }
        AreaSample sampleArea(Sampler &rng) const override
        {
            double theta = 2.0 * Pi * rng.next();           // Theta ranges from 0 to 2*Pi
            double phi = std::acos(2.0 * rng.next() - 1.0); // Phi ranges from 0 to Pi

            // Convert spherical coordinates to Cartesian coordinates
            double x = std::sin(phi) * std::cos(theta);
            double y = std::sin(phi) * std::sin(theta);
            double z = std::cos(phi);

            // Create a sample point on the unit sphere
            Point samplePoint(x, y, z);
            AreaSample result;
            result.position = samplePoint;
            populate(result, samplePoint);
            result.pdf = 1.0 / (4.0 * Pi);
            return result;
        }

        std::string toString() const override
        {
            return "Sphere[]";
        }
    };
}
REGISTER_SHAPE(Sphere, "sphere")