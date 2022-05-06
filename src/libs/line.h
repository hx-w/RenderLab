#ifndef LINE_H
#define LINE_H

#include "coords.h"

class Ray;

// 通过三个点确定三角面，计算三角面法线
Direction get_normal(const Point& p1, const Point& p2, const Point& p3);
// 通过射线与三角形三个顶点，确定是否相交，若相交，返回相交点
bool get_intersection(const Ray& ray, const Point& p1, const Point& p2, const Point& p3, Point& intersection);

class Ray {
public:
    Ray(): m_origin(0, 0, 0), m_direction(0, 0, 0) {}
    Ray(const Point& origin, const Direction& direction): m_origin(origin), m_direction(direction) {}
    Ray(const Ray& ray): m_origin(ray.m_origin), m_direction(ray.m_direction) {}
    ~Ray() {}
    Ray& operator=(const Ray& ray) {
        m_origin = ray.m_origin;
        m_direction = ray.m_direction;
        return *this;
    }

    Point get_point_by_t(Scalar t) const {
        return m_origin + m_direction * t;
    }

    Point& _get_origin() {
        return m_origin;
    }

    Direction& _get_direction() {
        return m_direction;
    }

    const Point& get_origin() const {
        return m_origin;
    }

    const Direction& get_direction() const {
        return m_direction;
    }
private:
    Point m_origin;
    Direction m_direction;
};


#endif
