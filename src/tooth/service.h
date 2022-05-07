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
    private:
        void _reset();

    private:
        int m_scale;
        FaceList m_faces;
        ToothEngine& m_engine;
    };
}

#endif
