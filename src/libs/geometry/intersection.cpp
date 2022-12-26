#include "intersection.h"
#include "mesh.h"
#include "line.hpp"

namespace geometry {
    using namespace std;

    bool intersect(
        const Ray& ray,
        const Mesh& mesh,
        vector<Point3f>& points,
        vector<Vector3f>& normals,
        bool only_first
    ) {
        // check all the faces
        auto faces = mesh.get_faces();
        auto vertices = mesh.get_vertices();
        auto dir = ray.get_direction();
        auto origin = ray.get_origin();

        auto EPSILON = 0.0000001f;
        Point3f cloest_point;
        Vector3f cloest_normal;
        float cloest_t = 1000000.f;

        for (auto face : faces) {
            auto v0 = vertices[face.x];
            auto v1 = vertices[face.y];
            auto v2 = vertices[face.z];

            auto e1 = v1 - v0;
            auto e2 = v2 - v0;
            auto p = glm::cross(dir, e2);
            auto a = glm::dot(e1, p);

            if (a > -EPSILON && a < EPSILON) {
                continue;
            }

            auto f = 1.f / a;
            auto s = origin - v0;
            auto u = f * glm::dot(s, p);

            if (u < 0.f || u > 1.f) {
                continue;
            }

            auto q = glm::cross(s, e1);
            auto v = f * glm::dot(dir, q);

            if (v < 0.f || u + v > 1.f) {
                continue;
            }

            auto t = f * glm::dot(e2, q);

            if (t > EPSILON) {
                auto point = ray.get_point(t);
                auto normal = glm::normalize(glm::cross(e1, e2));
                points.emplace_back(point);
                normals.emplace_back(normal);

                if (t < cloest_t) {
                    cloest_t = t;
                    cloest_point = point;
                    cloest_normal = normal;
                }
            }
        }

        if (only_first && points.size() > 0) {
            points.clear();
            normals.clear();
            points.emplace_back(cloest_point);
            normals.emplace_back(cloest_normal);
        }

        return points.size() > 0;
    }

}