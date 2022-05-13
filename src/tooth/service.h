#ifndef SERVICE_H
#define SERVICE_H

#include <vector>
#include <string>
#include "../libs/nurbs.h"
#include "../infrastructure/communication/ContextHub.h"

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
        
        template <class ...Args>
        static inline std::string fmt_str(const char* fmt, Args... args) {
            char buffer[1024]; // preset
            snprintf(buffer, sizeof(buffer), fmt, args...);
            return std::move(std::string(buffer));
        }

        // 对算表的不同情况进行处理
        void _table_handler(UVPoint& pivot, Scalar& dist, Point& tpnt, std::string& tface);

        void _draw_face(const std::string& name, int faceidx, const Point& clr);

        void _draw_arrow(const Point& p1, const Point& p2, const Point& clr);
    private:
        std::string m_name; // 名称
        int m_scale;
        FaceList m_faces;
        ToothEngine& m_engine;
    };
}

#endif
