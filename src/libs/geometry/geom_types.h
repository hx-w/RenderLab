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
    using Vector3u = glm::uvec3;

    using Vector2d = glm::dvec2;
    using Vector2f = glm::vec2;
    using Vector2u = glm::uvec2;

    using Mat4f = glm::mat4;

    class GeometryBase {
    public:
        GeometryBase() = default;
        virtual ~GeometryBase() = default;
    };

    class Line;
    class Ray;
    // class Triangle;
    // class Plane;
    // class Sphere;
    class Mesh;
}

#endif
