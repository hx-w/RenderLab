#ifndef NURBS_H
#define NURBS_H

/**
 * 离散NURBS曲面
 */

#include "surface.h"

class UVPoint: public Point {
public:
    UVPoint() = default;

private:
    UV uv;
};

class NURBS {
public:
    NURBS() = delete;
    NURBS(const Surface& surface) = delete;
    NURBS(const std::string& filename, int scale):
        m_surface(filename), m_scale(scale) {};
    ~NURBS() = default;

    Point get_point_by_uv(int iu, int iv) const;

    void cache_points();

private:
    Surface m_surface;
    int m_scale;  // uv都是整数，需要设置尺度
};

#endif
