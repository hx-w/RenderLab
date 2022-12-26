/**
 * intersection method between ray and mesh 
 */

#include <vector>
#include <glm/glm.hpp>
#include "types.h"


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

}