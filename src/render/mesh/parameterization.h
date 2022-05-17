#ifndef PARAMETERIZATION_H
#define PARAMETERIZATION_H

#include "elements.h"
#include <set>

namespace RenderSpace {
    class Parameterization {
    public:
        Parameterization() = default;
        Parameterization(MeshDrawable* ori, MeshDrawable* tar);

        ~Parameterization();

        void parameterize();

    private:
        // 标记ori面中的边缘点与非边缘点索引
        void _remark_vertices(std::set<int>&, std::set<int>&);

    private:
        MeshDrawable* m_ori;
        MeshDrawable* m_tar;
    };
}

#endif
