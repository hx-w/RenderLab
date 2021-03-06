#include "line.h"

Direction get_normal(const Point& p1, const Point& p2, const Point& p3) {
    Direction d1 = p2 - p1;
    Direction d2 = p3 - p1;
    return d1.cross(d2);
}

bool get_intersection(const Ray& ray, const Point& p1, const Point& p2, const Point& p3, Point& intersection) {
    Direction E1 = p2 - p1;
    Direction E2 = p3 - p1;
    Direction P = ray.get_direction().cross(E2);

    Scalar det = E1.dot(P);
    
    Direction T;
    if (det > 0.0) {
        T = ray.get_origin() - p1;
    }
    else {
        T = p1 - ray.get_origin();
        det = -det;
    }

    // 如果det接近0，也就是有向体积接近0，就说明射线和E1&E2平面共面。
    if (almostZero(det)) {
        return false;
    }

    // u, v
    Scalar u = Direction::dot(T, P);
    if (u < 0.0f || u > det) {
        return false;
    }

    Direction Q = Direction::cross(T, E1);

    Scalar v = Direction::dot(ray.get_direction(), Q);
    if (v < 0.0f || u + v > det) {
        return false;
    }

    Scalar t = Direction::dot(E2, Q);

    Scalar invD = 1.0f / det;
    t *= invD;

    intersection = ray.get_point_by_t(t);
    // or:
    // intersection = (1 - u - v)p1 + u * p2 + v * p3

    return true;
}
