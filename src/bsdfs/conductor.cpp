#include <lightwave.hpp>
#include <lightwave/math.hpp>
namespace lightwave {

class Conductor : public Bsdf {
    ref<Texture> m_reflectance;

public:
    Conductor(const Properties &properties) {
        m_reflectance = properties.get<Texture>("reflectance");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        // the probability of a light sample picking exactly the direction `wi'
        // that results from reflecting `wo' is zero, hence we can just ignore
        // that case and always return black
        // if(isReflection(wo, wi))
        //     return BsdfEval{.value = m_reflectance->evaluate(uv)};
        return BsdfEval::invalid();
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        BsdfSample b;
        b.wi = reflect(wo,Vector(0,0,1)).normalized(); // Reflect the incoming vector
        b.weight = m_reflectance->evaluate(uv);
        return b;
    }
    // bool isReflection(const Vector &wo, const Vector &wi) const {
    // // Assuming 'Frame' is a utility class that provides surface normal information
    //     // Vector no = - wi + 2.f * wi.dot(Vector(0,0,1)) * Vector(0,0,1);
    //     Vector normal = Vector(0,0,1); // Or another way to get the normal
    //     Vector perfectReflection = reflect(wo, normal);
    //     return (wi - perfectReflection).length() < 1e-8; // EPSILON is a small threshold value
    // }

    std::string toString() const override {
        return tfm::format("Conductor[\n"
                           "  reflectance = %s\n"
                           "]",
                           indent(m_reflectance));
    }
};

} // namespace lightwave

REGISTER_BSDF(Conductor, "conductor")
