#ifndef PARAMETERIZATION_H
#define PARAMETERIZATION_H

#include "elements.h"
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace RenderSpace {
    class Parameterization {
    public:
        Parameterization() = default;
        Parameterization(MeshDrawable* ori, MeshDrawable* tar);

        ~Parameterization();

        void parameterize();

    private:
        // 标记ori面中的边缘与非边缘
        void _remark_edges(std::vector<OrderedEdge>&, std::vector<OrderedEdge>&);
        void _topology_reorder(std::vector<OrderedEdge>&); // 对边缘边
        void _parameterize_bound(std::vector<OrderedEdge>&, std::vector<glm::vec2>&); // 参数平面 边缘点
    private:
        // 中间结果
        float m_bound_length; // 边缘总长度

    private:
        MeshDrawable* m_ori;
        MeshDrawable* m_tar;
    };
}

#endif
