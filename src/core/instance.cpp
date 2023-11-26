#include <lightwave/core.hpp>
#include <lightwave/instance.hpp>
#include <lightwave/registry.hpp>
#include <lightwave/sampler.hpp>

namespace lightwave {

void Instance::transformFrame(SurfaceEvent &surf) const {
    // hints:
    // * transform the hitpoint and frame here
    // * if m_flipNormal is true, flip the direction of the bitangent (which in effect flips the normal)
    // * make sure that the frame is orthonormal (you are free to change the bitangent for this, but keep
    //   the direction of the transformed tangent the same)
        surf.position = m_transform->apply(surf.position);
        surf.frame.tangent= m_transform->apply(surf.frame.tangent).normalized();
        Vector bitangent = m_transform->apply(surf.frame.bitangent);
        float proj = surf.frame.tangent.dot(bitangent);

        surf.frame.bitangent = (bitangent - proj * surf.frame.tangent).normalized();
        if (m_flipNormal)
        {
            surf.frame.bitangent = -surf.frame.bitangent;
        }

        surf.frame.normal = surf.frame.tangent.cross(surf.frame.bitangent).normalized();
        
}

bool Instance::intersect(const Ray &worldRay, Intersection &its, Sampler &rng) const {
    if (!m_transform) {
        // fast path, if no transform is needed
        Ray localRay = worldRay;
        if (m_shape->intersect(localRay, its, rng)) {
            its.instance = this;
            return true;
        } else {
            return false;
        }
    }
    const float previousT = its.t;
    Ray localRay;
    

    // hints:
    // * transform the ray (do not forget to normalize!)
    // * how does its.t need to change?
    //m_transform -> apply contains object - world
    localRay = m_transform->inverse(worldRay);
    float scaleNum = localRay.direction.length();
    localRay.direction = localRay.direction.normalized();
    its.t = its.t * scaleNum;  // its.t now contains the t in object space
    
    const bool wasIntersected = m_shape->intersect(localRay, its, rng);
    if (wasIntersected) {
        // hint: how does its.t need to change?

        its.instance = this;
        transformFrame(its);
        its.t = its.t / scaleNum;
        return true;
    } else {
        its.t = previousT;
        return false;
    }
}

Bounds Instance::getBoundingBox() const {
    if (!m_transform) {
        // fast path
        return m_shape->getBoundingBox();
    }

    const Bounds untransformedAABB = m_shape->getBoundingBox();
    if (untransformedAABB.isUnbounded()) {
        return Bounds::full();
    }

    Bounds result;
    for (int point = 0; point < 8; point++) {
        Point p = untransformedAABB.min();
        for (int dim = 0; dim < p.Dimension; dim++) {
            if ((point >> dim) & 1) {
                p[dim] = untransformedAABB.max()[dim];
            }
        }
        p = m_transform->apply(p);
        result.extend(p);
    }
    return result;
}

Point Instance::getCentroid() const {
    if (!m_transform) {
        // fast path
        return m_shape->getCentroid();
    }

    return m_transform->apply(m_shape->getCentroid());
}

AreaSample Instance::sampleArea(Sampler &rng) const {
    AreaSample sample = m_shape->sampleArea(rng);
    transformFrame(sample);
    return sample;
}

}

REGISTER_CLASS(Instance, "instance", "default")
