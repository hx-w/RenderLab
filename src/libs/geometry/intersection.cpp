#include "intersection.h"
#include "mesh.h"
#include "line.hpp"

#include <numeric>

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

        uint32_t cloest_ind = -1;
        float cloest_t = numeric_limits<float>::max();

        for (auto face : faces) {
            auto v0 = vertices[face.x];
            auto v1 = vertices[face.y];
            auto v2 = vertices[face.z];

            float t = 0.0f, u = 0.0f, v = 0.0f;

            if (!intersect_triangle(ray, v0, v1, v2, &t, &u, &v)) {
                continue;
            }

            auto _p = (1 - u - v) * v0 + u * v1 + v * v2;
            auto _n = glm::normalize(glm::cross(v1 - v0, v2 - v0));

            points.emplace_back(_p);
            normals.emplace_back(_n);

            auto abs_t = glm::abs(t);
            if (abs_t < cloest_t) {
                cloest_t = abs_t;
                cloest_ind = points.size() - 1;
            }
        }

        if (only_first && !points.empty()) {
            points.swap(vector<Point3f>{points[cloest_ind]});
            normals.swap(vector<Vector3f>{normals[cloest_ind]});
        }

        return !points.empty();
    }

    bool intersect_triangle(
        const Ray& ray,
        const Point3f& v0,
        const Point3f& v1,
        const Point3f& v2,
        float* t, float* u, float* v
    ) {
        // https://www.cnblogs.com/graphics/archive/2010/08/09/1795348.html
        const auto dir = ray.get_direction();
        const auto ori = ray.get_origin();

        auto E1 = v1 - v0;
        auto E2 = v2 - v0;
        auto P = glm::cross(dir, E2);

        // det
        float det = glm::dot(E1, P);

        auto T = ori - v0;
        if (det <= 0) {
            T = -T;
            det = -det;
        }

        if (det < 1e-6f) {
            return false;
        }

        *u = glm::dot(T, P);
        if (*u < 0.0f || *u > det) {
            return false;
        }

        auto Q = glm::cross(T, E1);

        *v = glm::dot(dir, Q);
        
        if (*v < 0.0f || *u + *v > det) {
            return false;
        }

        *t = glm::dot(E2, Q);
        
        float det_inv = 1.0f / det;

        *t *= det_inv;
        *u *= det_inv;
        *v *= det_inv;

        return true;
    }
}