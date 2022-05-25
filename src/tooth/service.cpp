#include "service.h"
#include "printer.h"
#include "execute.h"
#include <cmath>
#include <functional>
#include <array>
#include <string>

using namespace std;
using namespace fundamental;

namespace ToothSpace {
    ToothService::ToothService(
        ToothEngine& engine,
        const string& dir, int scale
    ) noexcept : m_engine(engine), m_autobus(make_unique<AutoBus>()) {
        _subscribe();
        _init(dir, scale);
    }

    ToothService::ToothService(ToothEngine& engine) noexcept
        : m_engine(engine) {}

    ToothService::~ToothService() {
        _reset();
        m_autobus.reset();
    }

    void ToothService::_init(const string& dir, int scale) {
        _reset();
        m_name = dir;
        m_scale = scale;
        string sep = "/";
#ifdef _WIN32
        sep = "\\";
#endif
        m_faces.emplace_back(NURBSFace(fmt_str(".%s%s%s%s", sep.c_str(), dir.c_str(), sep.c_str(), "face 1.txt"), scale, true));
        m_faces.emplace_back(NURBSFace(fmt_str(".%s%s%s%s", sep.c_str(), dir.c_str(), sep.c_str(), "face 2.txt"), scale, true));
        m_faces.emplace_back(NURBSFace(fmt_str(".%s%s%s%s", sep.c_str(), dir.c_str(), sep.c_str(), "face 3.txt"), scale, true));
        m_faces.emplace_back(NURBSFace(fmt_str(".%s%s%s%s", sep.c_str(), dir.c_str(), sep.c_str(), "face 4.txt"), scale, true));
    }

    void ToothService::_reset() {
        m_faces.clear();
        m_arrow_ids.clear();
        FaceList().swap(m_faces);
    }

    void ToothService::_subscribe() {
        // 拾取射线
        m_autobus->subscribe<void(const Point&, const Direction&)>(SignalPolicy::Sync, "render/picking_ray",
            bind(&ToothService::_pick_from_ray, this, placeholders::_1, placeholders::_2));
        // 清除拾取射线
        m_autobus->subscribe<void()>(SignalPolicy::Sync, "render/clear_picking",
            bind(&ToothService::_clear_arrow, this));
    }

    void ToothService::retag_point() {
        // 重设face1中边缘节点
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

    void ToothService::retag_point_by_ml() {
        // 对每个u计算预测边缘线v
        for (int iu = 0; iu < m_scale; ++iu) {
            // for valid check
            string cmd = "curl -s -G http://localhost:8848/api/edge_line/predict?u=" + to_string(iu * 1.0 / m_scale);
            string v_str = execute_short(cmd);
            int edge_v = static_cast<int>(stof(v_str) * m_scale);
            for (int iv = 0; iv < m_scale; ++iv) {
                if (iv < edge_v) {
                    m_faces[0]._cached_point(iu, iv)._type() = PointType::IN_EDGE;
                }
                else if (iv == edge_v) {
                    m_faces[0]._cached_point(iu, iv)._type() = PointType::ON_EDGE;
                }
                else {
                    m_faces[0]._cached_point(iu, iv)._type() = PointType::DEFAULT;
                }
            }
        }
    }

    void ToothService::simulate() {
        // 绘制face2-4完整面
        _draw_face(m_name + "[face-2]", 1, Point(1.0, 1.0, 0.0));
        _draw_face(m_name + "[face-3]", 2, Point(0.0, 1.0, 1.0));
        _draw_face(m_name + "[face-4]", 3, Point(0.7, 0.8, 0.5));
        // 创建网格
        int _face1_id = 0;
        int _target_id = 0;
        {
            auto _service = ContextHub::getInstance()->getService<int(const string&, int)>("render/create_mesh");
            _face1_id = _service.sync_invoke(m_name + "[face-1]", 2); // 三角网格
            _target_id = _service.sync_invoke(m_name + "[target]", 0); // 点图
        }

        // 原点 目标点 距离 目标面名称
        UVPoint pivot;
        Point tpnt;
        Scalar dist = 0.0;
        string tface = "";

        Printer saver("static/dataset/uv_pivot.csv");
        saver.to_csv("pos_u", "pos_v", "angle_u", "angle_v", "length_u", "length_v", "dist_x", "dist_y", "dist_z");

        for (int iu = 0; iu < m_scale; ++iu) {
            for (int iv = 0; iv < m_scale; ++iv) {
                pivot = m_faces[0]._cached_point(iu, iv);

                _dist_seek(pivot, dist, tpnt, tface);
   
                // 将原点与目标点绘制
                _send_uvpoint_to_render(pivot, tpnt, _face1_id, _target_id);
                // 将原点记录到数据集
                if (pivot.type() != PointType::DEFAULT)
                    continue; // 只收集到face2的点
                // 记录angle，因为是相对信息，用法线做记录可能需要前置条件一致
                const Direction& _dir_left = (iu == 0) ? m_faces[0].get_norm_by_uv(iu, iv) /* 临界点用法线做边界，只算了角度的一半 */ \
                    : (m_faces[0]._cached_point(iu - 1, iv) - pivot);
                const Direction& _dir_right = (iu == m_scale - 1) ? m_faces[0].get_norm_by_uv(iu, iv) \
                    : (m_faces[0]._cached_point(iu + 1, iv) - pivot);
                const Direction& _dir_top = (iv == 0) ? m_faces[0].get_norm_by_uv(iu, iv) \
                    : (m_faces[0]._cached_point(iu, iv - 1) - pivot);
                const Direction& _dir_bottom = (iv == m_scale - 1) ? m_faces[0].get_norm_by_uv(iu, iv) \
                    : (m_faces[0]._cached_point(iu, iv + 1) - pivot);
                Scalar _angle_u = _angle_between(_dir_left, _dir_right);
                Scalar _angle_v = _angle_between(_dir_top, _dir_bottom);
                if (iu == 0 || iu == m_scale - 1) _angle_u *= 2;
                if (iv == 0 || iv == m_scale - 1) _angle_v *= 2;

                // 计算length_u, length_v
                Scalar _length_u = (static_cast<int>(iu != 0) * _dir_left.mag() + static_cast<int>(iu != m_scale - 1) * _dir_right.mag()) / \
                    (static_cast<int>(iu != 0) + static_cast<int>(iu != m_scale - 1));
                Scalar _length_v = (static_cast<int>(iv != 0) * _dir_top.mag() + static_cast<int>(iv != m_scale - 1) * _dir_bottom.mag()) / \
                    (static_cast<int>(iv != 0) + static_cast<int>(iv != m_scale - 1));
                Direction dist_target = tpnt - pivot;
                saver.to_csv(iu * 1.0 / m_scale, iv * 1.0 / m_scale, _angle_u, _angle_v, _length_u, _length_v, dist_target.x(), dist_target.y(), dist_target.z());
            }
        }

        {
            auto _service = ContextHub::getInstance()->getService<void(int)>("render/refresh_mesh");
            _service.sync_invoke(_face1_id);
            _service.sync_invoke(_target_id);
        }

        // 保存边缘线
        _save_edgeline_to_csv("edge_line.csv");
        Printer::info(m_name + " -> done");
    }

    void ToothService::_dist_seek(UVPoint& pivot, Scalar& dist, Point& tpnt, string& tface) {
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
            else {
                // 在face1 边缘线上，但法线与face4无交，不设为SPECIAL
                // pivot._type() = PointType::SPECIAL;
                // tface = "SPECIAL";
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

    void ToothService::_send_uvpoint_to_render(UVPoint& pivot, Point& tpnt, int face1_id, int target_id) {
        const int iu = pivot.u();
        const int iv = pivot.v();
        Point _clr_pivot(0.0);
        Point _clr_target(0.0);
        if (pivot._type() == PointType::DEFAULT) {
        _clr_pivot = Point(1.0, 1.0, 1.0);
            _clr_target = Point(1.0, 1.0, 0.0);
        }
        else if (pivot._type() == PointType::ON_EDGE) {
            _clr_pivot = Point(0.0, 1.0, 0.0);
            _clr_target = Point(0.0, 1.0, 0.0);
        }
        else if (pivot._type() == PointType::IN_EDGE) {
            _clr_pivot = Point(0.0, 0.0, 1.0);
            _clr_target = Point(0.0, 0.0, 1.0);
        }
        else {
            _clr_pivot = Point(1.0, 0.0, 0.0);
            _clr_target = Point(1.0, 0.0, 0.0);
        }

        if (iu > 0 && iv > 0) {
            auto _service = ContextHub::getInstance()->getService<void(int, array<Point, 9>&&)>("render/add_triangle_raw");
            _service.sync_invoke(face1_id, array<Point, 9>{
                m_faces[0].get_point_by_uv(iu - 1, iv), _clr_pivot, m_faces[0].get_norm_by_uv(iu - 1, iv),
                m_faces[0].get_point_by_uv(iu, iv), _clr_pivot, m_faces[0].get_norm_by_uv(iu, iv),
                m_faces[0].get_point_by_uv(iu, iv - 1), _clr_pivot, m_faces[0].get_norm_by_uv(iu, iv - 1)
            });
            _service.sync_invoke(face1_id, array<Point, 9>{
                m_faces[0].get_point_by_uv(iu - 1, iv), _clr_pivot, m_faces[0].get_norm_by_uv(iu - 1, iv),
                m_faces[0].get_point_by_uv(iu, iv - 1), _clr_pivot, m_faces[0].get_norm_by_uv(iu, iv - 1),
                m_faces[0].get_point_by_uv(iu - 1, iv - 1), _clr_pivot, m_faces[0].get_norm_by_uv(iu - 1, iv - 1)
            });
        }

        // 设置target点
        if (pivot._type() != PointType::DEFAULT) {
            auto _service = ContextHub::getInstance()->getService<void(int, array<Point, 3>&&)>("render/add_vertex_raw");
            _service.sync_invoke(target_id, array<Point, 3>{
                tpnt, _clr_target, m_faces[0].get_norm_by_uv(iu, iv)
            });
        }
    }

    int ToothService::_draw_face(const string& name, int faceidx, const Point& clr) {
        // 预创建mesh
        int _face_id = 0;
        {
            auto _service = ContextHub::getInstance()->getService<int(const string&, int)>("render/create_mesh");
            _face_id = _service.sync_invoke(name, 2);
        }
        // 添加元素
        for (int iu = 1; iu < m_scale; ++iu) {
            for (int iv = 1; iv < m_scale; ++iv) {
                auto _service = ContextHub::getInstance()->getService<void(int, array<Point, 9>&&)>("render/add_triangle_raw");
                _service.sync_invoke(_face_id, array<Point, 9>{
                    m_faces[faceidx].get_point_by_uv(iu - 1, iv), clr, m_faces[faceidx].get_norm_by_uv(iu - 1, iv),
                    m_faces[faceidx].get_point_by_uv(iu, iv), clr, m_faces[faceidx].get_norm_by_uv(iu, iv),
                    m_faces[faceidx].get_point_by_uv(iu, iv - 1), clr, m_faces[faceidx].get_norm_by_uv(iu, iv - 1)
                });
                _service.sync_invoke(_face_id, array<Point, 9>{
                    m_faces[faceidx].get_point_by_uv(iu - 1, iv), clr, m_faces[faceidx].get_norm_by_uv(iu - 1, iv),
                    m_faces[faceidx].get_point_by_uv(iu, iv - 1), clr, m_faces[faceidx].get_norm_by_uv(iu, iv - 1),
                    m_faces[faceidx].get_point_by_uv(iu - 1, iv - 1), clr, m_faces[faceidx].get_norm_by_uv(iu - 1, iv - 1)
                });
            }
        }
        // 通知渲染器 更新
        {
            auto _service = ContextHub::getInstance()->getService<void(int)>("render/refresh_mesh");
            _service.sync_invoke(_face_id);
        }
        return _face_id;
    }

    int ToothService::_draw_arrow(const string& name, const Point& p1, const Point& p2, const Point& clr) {
        // 预创建mesh
        int _arrow_id = 0;
        {
            auto _service = ContextHub::getInstance()->getService<int(const string&, int)>("render/create_mesh");
            _arrow_id = _service.sync_invoke(name, 1);
        }
        // 添加元素
        {
            auto _service = ContextHub::getInstance()->getService<void(int, array<Point, 6>&&)>("render/add_edge_raw");
            _service.sync_invoke(_arrow_id, array<Point, 6>{
                p1, clr, Point(0.0), p2, clr, Point(1.0, 0.0, 0.0)
            });
        }

        // 通知渲染器 更新
        {
            auto _service = ContextHub::getInstance()->getService<void(int)>("render/refresh_mesh");
            _service.sync_invoke(_arrow_id);
        }
        return _arrow_id;
    }

    void ToothService::_clear_arrow() {
        // 通知渲染器 清除
        auto _service = ContextHub::getInstance()->getService<void(int)>("render/delete_mesh");
        for (auto& _id : m_arrow_ids) {
            _service.sync_invoke(_id);
        }
        m_arrow_ids.clear();
    }

    void ToothService::_pick_from_ray(const Point& ori, const Direction& dir) {
        Point picked_point(0.0);
        if (m_faces[0].get_intersection_by_ray(Ray(ori, dir), picked_point)) {
            Point f2tar(0.0), f3tar(0.0);
            Scalar f2dist = m_faces[1].get_nearest_point(picked_point, f2tar);
            Scalar f3dist = m_faces[2].get_nearest_point(picked_point, f3tar);
            Printer::info("PICKED in", m_name, "source:", picked_point, "f2dist:", f2dist, "f3dist:", f3dist);
            int _idf2 = _draw_arrow(m_name + "[f1_to_f2]", picked_point, f2tar, Point(0.8, 0.8, 0.0));
            int _idf3 = _draw_arrow(m_name + "[f1_to_f3]", picked_point, f3tar, Point(0.0, 0.8, 0.8));
            m_arrow_ids.push_back(_idf2);
            m_arrow_ids.push_back(_idf3);
        }
        else {
            Printer::info("NOT picked in", m_name);
        }
    }

    Scalar ToothService::_angle_between(const Direction& dir1, const Direction& dir2) {
        if (dir1.mag() == 0.0 || dir2.mag() == 0.0) {
            return 0.0;
        }
        return acos(dir1.dot(dir2) / (dir1.mag() * dir2.mag()));
    }

    void ToothService::_save_edgeline_to_csv(const string& filename) {
        Printer printer("static/dataset/" + filename);
        printer.to_csv("u", "v");
        for (int iu = 0; iu < m_scale; ++iu) {
            for (int iv = 0; iv < m_scale; ++iv) {
                UVPoint& pivot = m_faces[0]._cached_point(iu, iv);
                if (pivot.type() == PointType::ON_EDGE) {
                    printer.to_csv(iu * 1.0 / m_scale, iv * 1.0 / m_scale);
                }
            }
        }
    }
}
