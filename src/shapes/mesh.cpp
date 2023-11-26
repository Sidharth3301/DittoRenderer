#include <lightwave.hpp>

#include "../core/plyparser.hpp"
#include "accel.hpp"

namespace lightwave
{

    /**
     * @brief A shape consisting of many (potentially millions) of triangles, which share an index and vertex buffer.
     * Since individual triangles are rarely needed (and would pose an excessive amount of overhead), collections of
     * triangles are combined in a single shape.
     */
    class TriangleMesh : public AccelerationStructure
    {
        /**
         * @brief The index buffer of the triangles.
         * The n-th element corresponds to the n-th triangle, and each component of the element corresponds to one
         * vertex index (into @c m_vertices ) of the triangle.
         * This list will always contain as many elements as there are triangles.
         */
        std::vector<Vector3i> m_triangles;
        /**
         * @brief The vertex buffer of the triangles, indexed by m_triangles.
         * Note that multiple triangles can share vertices, hence there can also be fewer than @code 3 * numTriangles @endcode
         * vertices.
         */
        std::vector<Vertex> m_vertices;
        /// @brief The file this mesh was loaded from, for logging and debugging purposes.
        std::filesystem::path m_originalPath;
        /// @brief Whether to interpolate the normals from m_vertices, or report the geometric normal instead.
        bool m_smoothNormals;

    protected:
        int numberOfPrimitives() const override
        {
            return int(m_triangles.size());
        }

        bool intersect(int primitiveIndex, const Ray &ray, Intersection &its, Sampler &rng) const override
        {

            // hints:
            // * use m_triangles[primitiveIndex] to get the vertex indices of the triangle that should be intersected
            // * if m_smoothNormals is true, interpolate the vertex normals from m_vertices
            //   * make sure that your shading frame stays orthonormal!
            // * if m_smoothNormals is false, use the geometrical normal (can be computed from the vertex positions)

            // mullard trumbore
            Vector3i triangle = m_triangles[primitiveIndex];
            Vertex A = m_vertices[triangle[0]];
            Vertex B = m_vertices[triangle[1]];
            Vertex C = m_vertices[triangle[2]];
            Vector v0 = (Vector)A.position;
            Vector v1 = (Vector)B.position;
            Vector v2 = (Vector)C.position;

            Vector edge1, edge2, T, P, Q;
            // following scratchpixel
            edge1 = v1 - v0;
            edge2 = v2 - v0;
            P = ray.direction.cross(edge2);
            float det = P.dot(edge1);
            Vector N = edge1.cross(edge2).normalized();
            // for mesh inside to pass, yes we need backward facing intersections as well
            if (fabs(det) < 1e-8) //for this we need a different epsilon than the self intersection which is quite loose
                return false;     // inputs from tut on 23/11
            T = Vector(ray.origin) - v0;
            Q = T.cross(edge1);

            float denom = 1 / det;
            float u = P.dot(T) * denom;
            if (u < 0.f || u > 1.f)
                return false;
            float v = Q.dot(ray.direction) * denom;
            if (v < 0.f || u + v > 1.f)
                return false;

            Vector hitPoint = (1 - u - v) * v0 + u * v1 + v * v2;
            float t = Q.dot(edge2) * denom;

            // we take the closest t, and if the ray intersects a triangle behind the one we just did, then its
            // not visible to the camera
            if (t > its.t)
            {
                return false;
            }
            if ((Vector(ray.origin) - hitPoint).length() < Epsilon)
            { // self intersection check
                return false;
            }
            its.t = t;
            // populate(its, ray(its.t));
            Vector2 bary{u, v};
            if (m_smoothNormals == true)
            {
                Vertex barycentric_normal = Vertex::interpolate(bary, A, B, C);
                its.frame.normal = barycentric_normal.normal.normalized();
                its.frame = Frame(its.frame.normal);           
            }
            else
            {
                its.frame.normal = N;
                its.frame = Frame(its.frame.normal);
            }
            its.uv.x() = u;
            its.uv.y() = v;
            return true;
        }

        Bounds getBoundingBox(int primitiveIndex) const override
        {
            // m_vertices.at(0)
            Vector3i triangle = m_triangles[primitiveIndex];
            Vertex v1 = m_vertices[triangle[0]];
            Vertex v2 = m_vertices[triangle[1]];
            Vertex v3 = m_vertices[triangle[2]];

            Point b0 = elementwiseMin(elementwiseMin(v1.position, v2.position), v3.position);
            Point b1 = elementwiseMax(elementwiseMax(v1.position, v2.position), v3.position);
            return Bounds(b0, b1);
        }

        Point getCentroid(int primitiveIndex) const override
        {
            Vector3i triangle = m_triangles[primitiveIndex];
            Vector v1 = (Vector)m_vertices[triangle[0]].position;
            Vector v2 = (Vector)m_vertices[triangle[1]].position;
            Vector v3 = (Vector)m_vertices[triangle[2]].position;
            return Vector((v1 + v2 + v3)/ 3.0);
        }

    public:
        TriangleMesh(const Properties &properties)
        {
            m_originalPath = properties.get<std::filesystem::path>("filename");
            m_smoothNormals = properties.get<bool>("smooth", true);
            readPLY(m_originalPath.string(), m_triangles, m_vertices);
            logger(EInfo, "loaded ply with %d triangles, %d vertices",
                   m_triangles.size(),
                   m_vertices.size());
            buildAccelerationStructure();
        }

        AreaSample sampleArea(Sampler &rng) const override{
            // only implement this if you need triangle mesh area light sampling for your rendering competition
            NOT_IMPLEMENTED}

        std::string toString() const override
        {
            return tfm::format(
                "Mesh[\n"
                "  vertices = %d,\n"
                "  triangles = %d,\n"
                "  filename = \"%s\"\n"
                "]",
                m_vertices.size(),
                m_triangles.size(),
                m_originalPath.generic_string());
        }
    };

}

REGISTER_SHAPE(TriangleMesh, "mesh")
