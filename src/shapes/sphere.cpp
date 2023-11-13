#include <lightwave.hpp>
#include <ostream>
namespace lightwave
{
    class Sphere : public Shape
    {
        inline void populate(SurfaceEvent &surf, const Point &position) const
        {
            surf.position = position;

            // map the position from [-1,-1,0]..[+1,+1,0] to [0,0]..[1,1] by discarding the z component and rescaling
            surf.uv.x() = (position.x() + 1) / 2;
            surf.uv.y() = (position.y() + 1) / 2;
            surf.frame.normal = (position - Point(0., 0., 0.)).normalized();

            surf.frame.tangent = surf.frame.normal.cross(Vector(0, 1, 0));
            surf.frame.bitangent = surf.frame.normal.cross(surf.frame.tangent);
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
            logger(EDebug, "hitpoint rn at %.2f", its.t);
            float t0, t1;
            if (discriminant < 0)
                return false;
            else if (discriminant == 0)
                t0 = t1 = -0.5 * b / a;
            else
            {
                float q = (b > 0) ? -0.5 * (b + sqrt(discriminant)) : -0.5 * (b - sqrt(discriminant));
                t0 = q / a;
                t1 = c / q;
            }
            if (t0 < Epsilon)
                return false;
            if (t0 > t1)
            {
                float tmp = t0;
                t0 = t1;
                t1 = tmp;
            }
            if (t0 < 0)
            {
                t0 = t1; // if t0 is negative, let's use t1 instead
                if (t0 < 0)
                    return false; // both t0 and t1 are negative
            }

            its.t = t0;
            logger(EDebug, "hitpoint rn at %.2f", its.t);
            
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
        AreaSample sampleArea(Sampler &rng) const override{
            NOT_IMPLEMENTED}

        std::string toString() const override
        {
            return "Sphere[]";
        }
    };
}
REGISTER_SHAPE(Sphere, "sphere")