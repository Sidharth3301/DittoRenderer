#include "fresnel.hpp"
#include <lightwave.hpp>

namespace lightwave {

class Dielectric : public Bsdf {
    ref<Texture> m_ior;
    ref<Texture> m_reflectance;
    ref<Texture> m_transmittance;

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
        Vector normal = Vector(0,0,1); // Obtain the correct surface normal
        float ior = m_ior->evaluate(uv).g; // Assuming IOR is a single value texture

        bool entering = Frame::cosTheta(wo) > 0;
        float eta = entering ? 1/ ior : ior; // Adjust IOR based on ray direction

        Vector reflectDir = reflect(wo, normal).normalized();
        Vector refractDir;
        float fresnelReflectance = fresnelDielectric(wo, normal, eta, refractDir);

        BsdfSample bsdfSample;
        if (rng.next() < fresnelReflectance || !refract(wo, normal, eta, refractDir)) {
            // Reflect
            bsdfSample.wi = reflect(wo,Vector(0,0,1)).normalized();
            bsdfSample.weight = m_reflectance->evaluate(uv);

        } else {
            // Refract
            bsdfSample.wi =reflect(wo,Vector(0,0,1)).normalized();
            bsdfSample.weight = m_transmittance->evaluate(uv);
        }

        return bsdfSample;
    }
    bool refract(const Vector &wo, const Vector &normal, float eta, Vector &refractDir) const {
        float cosThetaI = normal.dot(wo);
        float sin2ThetaI = std::max(0.0f, 1.0f - cosThetaI * cosThetaI);
        float sin2ThetaT = eta * eta * sin2ThetaI;

        // Check for total internal reflection
        if (sin2ThetaT >= 1.0f) return false;

        float cosThetaT = std::sqrt(1.0f - sin2ThetaT);
        refractDir = eta * -wo + (eta * cosThetaI - cosThetaT) * normal;
        return true;
    }
    float fresnelDielectric(const Vector &wo, const Vector &normal, float eta, Vector &refractDir)  const {
        float cosThetaI = normal.dot(wo);
        float sin2ThetaI = std::max(0.0f, 1.0f - cosThetaI * cosThetaI);
        float sin2ThetaT = eta * eta * sin2ThetaI;

        // Total internal reflection
        if (sin2ThetaT >= 1.0f) return 1.0f;

        float cosThetaT = std::sqrt(1.0f - sin2ThetaT);

        float rParl = ((eta * cosThetaI) - cosThetaT) / ((eta * cosThetaI) + cosThetaT);
        float rPerp = (cosThetaI - (eta * cosThetaT)) / (cosThetaI + (eta * cosThetaT));
        return 0.5f * (rParl * rParl + rPerp * rPerp);
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
