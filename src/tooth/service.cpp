#include "service.h"
#include "printer.h"

using namespace std;

namespace ToothSpace {
    ToothService::ToothService(
        ToothEngine& engine,
        const string& dir, int scale
    ) noexcept : m_engine(engine) {
        _init(dir, scale);
    }

    ToothService::ToothService(ToothEngine& engine) noexcept
        : m_engine(engine) {}

    ToothService::~ToothService() {
        _reset();
    }

    void ToothService::_init(const string& dir, int scale) {
        _reset();
        m_scale = scale;
        m_faces.emplace_back(NURBSFace(dir + "/face-1.txt", scale, true));
        m_faces.emplace_back(NURBSFace(dir + "/face-2.txt", scale, true));
        m_faces.emplace_back(NURBSFace(dir + "/face-3.txt", scale, true));
        m_faces.emplace_back(NURBSFace(dir + "/face-4.txt", scale, true));
    }

    void ToothService::_reset() {
        m_faces.clear();
        FaceList().swap(m_faces);
    }

    void ToothService::refresh_edge() {
        // 重设face1中边缘节点
        int count = 0;
        for (int iu = 0; iu < m_scale; ++iu) {
            for (int iv = 0; iv < m_scale; ++iv) {
                Point _t;
                UVPoint& f1pnt = m_faces[0]._cached_point(iu, iv);
                Scalar f2dist = m_faces[1].get_nearest_point(f1pnt, _t);
                Scalar f3dist = m_faces[2].get_nearest_point(f1pnt, _t);
                if (f3dist < f2dist || almostEqual(f3dist, f2dist)) {
                    f1pnt._type() = PointType::IN_EDGE;
                }
            }
        }
        // 去除uv边缘
        for (int iu = 1; iu < m_scale - 1; ++iu) {
            for (int iv = 1; iv < m_scale - 1; ++iv) {
                UVPoint& f1pnt = m_faces[0]._cached_point(iu, iv);
                if (f1pnt._type() != PointType::IN_EDGE) {
                    continue;
                }
                // f1pnt 不在uv边缘，且四周存在DEFAULT，则设为ON_EDGE
                if (
                    m_faces[0]._cached_point(iu - 1, iv).type() != PointType::DEFAULT &&
                    m_faces[0]._cached_point(iu + 1, iv).type() != PointType::DEFAULT &&
                    m_faces[0]._cached_point(iu, iv - 1).type() != PointType::DEFAULT &&
                    m_faces[0]._cached_point(iu, iv + 1).type() != PointType::DEFAULT
                ) {
                    f1pnt._type() = PointType::ON_EDGE;
                }
            }
        }
    }
}