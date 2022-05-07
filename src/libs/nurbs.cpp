#include "nurbs.h"
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
        return m_points[iu][iv];
    }
    return move(m_surface.get_point_by_uv(_itof(iu), _itof(iv)));
}

void NURBSFace::cache_points() {
    _pfree();
    UV uv_max = m_surface.get_degree();
    m_points.resize(uv_max.first);
    for (int iu = 0; iu < uv_max.first; ++iu) {
        m_points[iu].resize(uv_max.second);
        for (int iv = 0; iv < uv_max.second; ++iv) {
            m_points[iu][iv] = UVPoint(get_point_by_uv(iu, iv), iu, iv);
        }
    }
    m_is_cached = true;
}
