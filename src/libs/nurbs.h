#ifndef NURBS_H
#define NURBS_H

/**
 * 离散NURBSFace曲面
 */
#include "surface.h"

using std::vector;

enum class PointType {
    DEFAULT,
    ON_EDGE,
    IN_EDGE
};

// UVPoint 需要在NURBSFace环境下使用才有意义
class UVPoint: public Point {
public:
    UVPoint() = default;
    UVPoint(Scalar x, Scalar y, Scalar z, Scalar u, Scalar v):
        Point(x, y, z), m_uv(UV(u, v)), m_type(PointType::DEFAULT) {}
    UVPoint(const Point& p, Scalar u, Scalar v):
        Point(p), m_uv(UV(u, v)) {}
    
    int u() const { return m_uv.first; }
    int v() const { return m_uv.second; }
    PointType type() const { return m_type; }

    int& _u() { return m_uv.first; }
    int& _v() { return m_uv.second; }
    PointType& _type() { return m_type; }

    bool operator== (const UVPoint& p) const {
        return Point::operator==(p) && m_uv == p.m_uv;
    }

private:
    UV m_uv;  // iuv
    PointType m_type;
};

class NURBSFace {
public:
    NURBSFace() = default;
    NURBSFace(const Surface& surface) = delete;
    NURBSFace(const std::string& filename, int scale, bool pre_cache=false) {
        init(filename, scale, pre_cache);
    };
    ~NURBSFace();

    void init(const std::string& filename, int scale, bool pre_cache=false);

    void cache_points();

    Point get_point_by_uv(int iu, int iv) const;
    Direction get_norm_by_uv(int iu, int iv) const;
    Scalar get_nearest_point(const Point& pnt, Point& ret) const;

    UVPoint& _cached_point(int iu, int iv);
private:
    void _pfree();
    inline Scalar _itof(int i) const {
        return i * 1.0 / m_scale;
    }

private:
    Surface m_surface;
    int m_scale;  // uv都是整数，需要设置尺度
    
    bool m_is_cached = false;
    vector<vector<UVPoint>> m_points; // uv缓存点
};

#endif
