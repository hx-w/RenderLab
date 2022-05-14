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

        // 计算结果 保存到csv
        void simulate(const std::string& target);

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
        void _table_handler(UVPoint& pivot, Scalar& dist, Point& tpnt, std::string& tface);

        void _pick_from_ray(const Point& ori, const Direction& dir);

        int _draw_face(const std::string& name, int faceidx, const Point& clr);
        int _draw_arrow(const std::string& name, const Point& p1, const Point& p2, const Point& clr);

        void _clear_arrow();
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
