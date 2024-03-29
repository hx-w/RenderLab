/**
 * intersection method between ray and mesh 
 */

#include <vector>
#include <glm/glm.hpp>
#include "geom_types.h"


namespace geometry {

    /// @brief intersect between ray and mesh
    /// @param ray the ray
    /// @param mesh the mesh
    /// @param points the intersection points
    /// @param normals the normals of the intersection points
    /// @param only_first if true, only return the first intersection point
    bool intersect(
        const Ray&,
        const Mesh&,
        std::vector<Point3f>&,
        std::vector<Vector3f>&,
        bool
    );


    bool intersect_triangle(
        const Ray&,
        const geometry::Point3f& v0,
        const geometry::Point3f& v1,
        const geometry::Point3f& v2,
        float* t, float* u, float* v
    );
}