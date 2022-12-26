/**
 * define the types used in the geometry library
 * combine with glm 
 */

#ifndef GEOMETRY_TYPES_H
#define GEOMETRY_TYPES_H


#include <glm/glm.hpp>

namespace geometry {
    using Point3d = glm::dvec3;
    using Point3f = glm::vec3;

    using Vector3d = glm::dvec3;
    using Vector3f = glm::vec3;

    using Coord3d = glm::dvec3;
    using Coord3f = glm::vec3;

    class Line;
    class Ray;
    // class Triangle;
    // class Plane;
    // class Sphere;
    class Mesh;
}

#endif
