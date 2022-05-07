#ifndef SERVICE_H
#define SERVICE_H

#include <vector>
#include <string>
#include "../libs/nurbs.h"

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
        void refresh_edge();

        // 计算结果 保存到csv
        void calculate_table(const std::string& target);
    private:
        void _reset();
        
        template <class ...Args>
        static inline std::string fmt_str(const char* fmt, Args... args) {
            char buffer[1024]; // preset
            sprintf(buffer, fmt, args...);
            return std::move(std::string(buffer));
        }

        // 对算表的不同情况进行处理
        void _table_handler(UVPoint& pivot, Scalar& dist, Point& tpnt, std::string& tface);

    private:
        int m_scale;
        FaceList m_faces;
        ToothEngine& m_engine;
    };
}

#endif
