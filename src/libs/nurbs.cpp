#include "nurbs.h"
#include "line.h"

using namespace std;

NURBSFace::~NURBSFace() {
    _pfree();
}

void NURBSFace::_pfree() {
    m_is_cached = false;
    vector<vector<UVPoint>>().swap(m_points);
}

void NURBSFace::init(const string& filename, int scale, bool pre_cache) {
    m_surface.init(filename);
    m_scale = scale;
    if (pre_cache) {
        cache_points();
    }
}

Point NURBSFace::get_point_by_uv(int iu, int iv) const {
    if (m_is_cached) {
        return m_points.at(iu).at(iv);
    }
    return move(m_surface.get_point_by_uv(_itof(iu), _itof(iv)));
}

Direction NURBSFace::get_norm_by_uv(int iu, int iv) const {
    return move(m_surface.get_normal_by_uv(_itof(iu), _itof(iv), 1.0/m_scale));
}

Scalar NURBSFace::get_nearest_point(const Point& pnt, Point& ret) const {
    Scalar min_dist = __FLT_MAX__;
    for (int iu = 0; iu < m_scale; ++iu) {
        for (int iv = 0; iv < m_scale; ++iv) {
            Point cpnt = m_is_cached ? \
                static_cast<Point>(m_points.at(iu).at(iv)):\
                get_point_by_uv(iu, iv);
            Scalar c_dist = cpnt.dist(pnt);
            if (c_dist < min_dist) {
                min_dist = c_dist;
                ret = cpnt;
            }
        }
    }
    return min_dist;
}

void NURBSFace::cache_points() {
    _pfree();
    m_points.resize(m_scale);
    for (int iu = 0; iu < m_scale; ++iu) {
        m_points[iu].resize(m_scale);
        for (int iv = 0; iv < m_scale; ++iv) {
            m_points[iu][iv] = UVPoint(get_point_by_uv(iu, iv), iu, iv);
        }
    }
    m_is_cached = true;
}

UVPoint& NURBSFace::_cached_point(int iu, int iv) {
    if (!m_is_cached) {
        cache_points();
    }
    return m_points.at(iu).at(iv);
}

bool NURBSFace::get_intersection_by_ray(const Ray& ray, Point& ipnt) const {
    /**
     *  [0, 0]  [0, 1] ... [0, m_scale-1]
     *  [1, 0]  [1, 1] ... [1, m_scale-1]
     *  将四边形分解成左上/右下两个三角形
     */
    for (int iu = 1; iu < m_scale; ++iu) {
        for (int iv = 1; iv < m_scale; ++iv) {
            if (get_intersection(
                ray,
                get_point_by_uv(iu - 1, iv - 1),
                get_point_by_uv(iu, iv - 1),
                get_point_by_uv(iu - 1, iv),
                ipnt
            )) { return true; }
            if (get_intersection(
                ray,
                get_point_by_uv(iu, iv),
                get_point_by_uv(iu, iv - 1),
                get_point_by_uv(iu - 1, iv),
                ipnt
            )) { return true; }
        }
    }
    return false;
}