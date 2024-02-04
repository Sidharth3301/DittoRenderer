#include "fresnel.hpp"
#include <lightwave.hpp>

namespace lightwave {

class Dielectric : public Bsdf {
    ref<Texture> m_ior;
    ref<Texture> m_reflectance;
    ref<Texture> m_transmittance;
    // Color albedo;

public:
    Dielectric(const Properties &properties) {
        m_ior           = properties.get<Texture>("ior");
        m_reflectance   = properties.get<Texture>("reflectance");
        m_transmittance = properties.get<Texture>("transmittance");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        // the probability of a light sample picking exactly the direction `wi'
        // that results from reflecting or refracting `wo' is zero, hence we can
        // just ignore that case and always return black
        return BsdfEval::invalid();
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {

        float ior = m_ior->scalar(uv); // Assuming IOR is a single value texture
        auto normal = Vector(0,0,1);
        bool entering = Frame::cosTheta(wo) < 0;
        float eta = entering ? (1/ ior) : ior; 
        auto adj_normal = entering ? -1*normal: normal;// Adjust IOR based on ray direction
        float cosThetaI = Frame::cosTheta(wo); 
        float fresnelReflectance = fresnelDielectric(cosThetaI, eta);

        BsdfSample bsdfSample;
        if (rng.next() < fresnelReflectance){ 
            // Reflect
            bsdfSample.wi = reflect(wo,adj_normal).normalized();
            bsdfSample.weight = m_reflectance->evaluate(uv);

        } else {
            // Refract
            
            bsdfSample.wi = refract(wo,adj_normal, eta).normalized();
            bsdfSample.weight = m_transmittance->evaluate(uv)/(eta*eta);
        }
        
        return bsdfSample;
    }
    Color getAlbedo(const Point2 &uv, const Vector &wo) const override {
        float ior = m_ior->scalar(uv); // Assuming IOR is a single value texture
        // auto normal = Vector(0,0,1);
        bool entering = Frame::cosTheta(wo) < 0;
        float eta = entering ? (1/ ior) : ior; 
        return (m_reflectance->evaluate(uv)+m_transmittance->evaluate(uv)/(eta*eta))*0.5;
    }
   
    std::string toString() const override {
        return tfm::format("Dielectric[\n"
                           "  ior           = %s,\n"
                           "  reflectance   = %s,\n"
                           "  transmittance = %s\n"
                           "]",
                           indent(m_ior), indent(m_reflectance),
                           indent(m_transmittance));
    }
};

} // namespace lightwave

REGISTER_BSDF(Dielectric, "dielectric")
