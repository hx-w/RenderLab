#ifndef SERVICE_H
#define SERVICE_H

#include <vector>
#include <string>
#include <memory>
#include "../libs/nurbs.h"
#include "../infrastructure/communication/ContextHub.h"
#include "../infrastructure/communication/AutoBus.hpp"

namespace ToothSpace {
    class ToothEngine;

    typedef std::vector<NURBSFace> FaceList;

    class ToothService {
    public:
        ToothService() = delete;
        explicit ToothService(
            ToothEngine& engine,
            const std::string& dir, int scale=100
        ) noexcept;
        explicit ToothService(ToothEngine& engine) noexcept;

        ~ToothService();

        void _init(const std::string& dir, int scale);
    
        // 对face1，计算边缘点，更新点种类
        void retag_point();

        // 根据机器学习模型，计算每个点的类型
        void retag_point_by_ml();

        // 计算结果 保存到csv
        void simulate();

        void simulate_by_ml();

    private:
        void _reset();

        void _subscribe();
        
        template <class ...Args>
        static inline std::string fmt_str(const char* fmt, Args... args) {
            char buffer[1024]; // preset
            snprintf(buffer, sizeof(buffer), fmt, args...);
            return std::move(std::string(buffer));
        }

        // 对算表的不同情况进行处理
        void _dist_seek(UVPoint& pivot, Scalar& dist, Point& tpnt, std::string& tface);

        void _send_uvpoint_to_render(UVPoint& pivot, Point& tpnt, int face1_id, int target_id, bool show_default=false);

        void _pick_from_ray(const Point& ori, const Direction& dir);

        int _draw_face(const std::string& name, int faceidx, const Point& clr);
        int _draw_arrow(const std::string& name, const Point& p1, const Point& p2, const Point& clr);

        void _clear_arrow();

        Scalar _angle_between(const Direction& d1, const Direction& d2);

        void _save_edgeline_to_csv(const std::string& target);

        // 获取face1上的angle_u/angle_v/length_u/length_v
        void _get_face1_info(const UVPoint& pivot, Scalar& angle_u, Scalar& angle_v, Scalar& length_u, Scalar& length_v);

    private:
        std::string m_name; // 名称
        int m_scale;
        FaceList m_faces;
        ToothEngine& m_engine;

        std::vector<int> m_arrow_ids;

        std::unique_ptr<fundamental::AutoBus> m_autobus;
    };
}

#endif
