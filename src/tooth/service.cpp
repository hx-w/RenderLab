#include "service.h"
#include "printer.h"
#include <functional>

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
        string sep = "/";
#ifdef _WIN32
        sep = "\\";
#endif
        m_faces.emplace_back(NURBSFace(dir + sep + "face-1.txt", scale, true));
        m_faces.emplace_back(NURBSFace(dir + sep + "face-2.txt", scale, true));
        m_faces.emplace_back(NURBSFace(dir + sep + "face-3.txt", scale, true));
        m_faces.emplace_back(NURBSFace(dir + sep + "face-4.txt", scale, true));
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
                    f1pnt._type() = PointType::ON_EDGE;
                }
                else {
                    f1pnt._type() = PointType::DEFAULT;
                }
            }
        }
        // 去除uv边缘
        for (int iu = 1; iu < m_scale - 1; ++iu) {
            for (int iv = 1; iv < m_scale - 1; ++iv) {
                UVPoint& f1pnt = m_faces[0]._cached_point(iu, iv);
                if (f1pnt._type() != PointType::ON_EDGE) {
                    continue;
                }
                // f1pnt 不在uv边缘，且四周存在DEFAULT，则设为IN_EDGE
                if (
                    m_faces[0]._cached_point(iu - 1, iv).type() != PointType::DEFAULT &&
                    m_faces[0]._cached_point(iu + 1, iv).type() != PointType::DEFAULT &&
                    m_faces[0]._cached_point(iu, iv - 1).type() != PointType::DEFAULT &&
                    m_faces[0]._cached_point(iu, iv + 1).type() != PointType::DEFAULT
                ) {
                    f1pnt._type() = PointType::IN_EDGE;
                }
            }
        }
    }

    void ToothService::calculate_table(const string& target) {
        Printer saver(target);
        saver.to_csv(
            "source uv", "source xyz", 
            "target xyz", "target L",
            "target face"
        );

        // 原点 目标点 距离 目标面名称
        UVPoint pivot;
        Point tpnt;
        Scalar dist = 0.0;
        string tface = "";

        for (int iu = 0; iu < m_scale; ++iu) {
            for (int iv = 0; iv < m_scale; ++iv) {
                pivot = m_faces[0]._cached_point(iu, iv);

                _table_handler(pivot, dist, tpnt, tface);

                saver.to_csv(
                    fmt_str("\"%.2lf,%.2lf\"", iu * 1.0 / m_scale, iv * 1.0 / m_scale),
                    fmt_str("\"%lf,%lf,%lf\"", pivot.x(), pivot.y(), pivot.z()),
                    fmt_str("\"%lf,%lf,%lf\"", tpnt.x(), tpnt.y(), tpnt.z()),
                    dist, tface
                );
            }
        }
    }

    void ToothService::_table_handler(UVPoint& pivot, Scalar& dist, Point& tpnt, string& tface) {
        const int iu = pivot.u();
        const int iv = pivot.v();
        if (pivot.type() == PointType::UNKNOWN) {
            // todo
        }
        // 默认取face2最近距离点
        else if (pivot.type() == PointType::DEFAULT) {
            tface = "DEFAULT";
            dist = m_faces[1].get_nearest_point(pivot, tpnt);
        }
        // 边缘线上的点，取face4与边缘点法线的交点
        else if (pivot.type() == PointType::ON_EDGE) {
            tface = "ON_EDGE";
            Direction _norm = m_faces[0].get_norm_by_uv(iu, iv);
            // 与face4 求交
            bool rb = m_faces[3].get_intersection_by_ray(Ray(static_cast<Point>(pivot), _norm), tpnt);
            if (rb) {
                dist = pivot.dist(tpnt);
            }
            else { // 在face1 边缘线上，但法线与face4无交
                pivot._type() = PointType::SPECIAL;
                tface = "SPECIAL";
                dist = m_faces[3].get_nearest_point(pivot, tpnt);
            }
        }
        // 属于边缘点，但不在边缘线上，需要找到所有可选的边缘线上的法线进行求交
        else if (pivot.type() == PointType::IN_EDGE) {
            tface = "IN_EDGE";
            Direction _norm;
            // 将周围边缘线上点的法线累加起来
            auto func = [this, &_norm](int u, int v) mutable {
                if (this->m_faces[0]._cached_point(u, v)._type() == PointType::ON_EDGE) {
                    _norm += m_faces[0].get_norm_by_uv(u, v);
                }
            };
            // 沿四个方向找点
            for (int _up = 1; iu - _up >= 0; ++_up) {
                func(iu - _up, iv);
            }
            for (int _down = 1; iu + _down < m_scale; ++_down) {
                func(iu + _down, iv);
            }
            for (int _left = 1; iv - _left >= 0; ++_left) {
                func(iu, iv - _left);
            }
            for (int _right = 1; iv + _right < m_scale; ++_right) {
                func(iu, iv + _right);
            }
            _norm.normalize();
            // 如果法线不存在
            if (_norm == Direction()) {
                // 求face4最近点
                tface = "SPECIAL";
                pivot._type() = PointType::SPECIAL;
                dist = m_faces[3].get_nearest_point(pivot, tpnt);
            }
            else {
                bool rb = m_faces[3].get_intersection_by_ray(Ray(static_cast<Point>(pivot), _norm), tpnt);
                if (rb) {
                    dist = pivot.dist(tpnt);
                }
                else { // 在face1 边缘，但法线与face4无交
                    tface = "SPECIAL";
                    pivot._type() = PointType::SPECIAL;
                    dist = m_faces[3].get_nearest_point(pivot, tpnt);
                }
            }
        }
        else {
            // error type
            Printer::to_console("error: invalid point type");
        }
    }
}
