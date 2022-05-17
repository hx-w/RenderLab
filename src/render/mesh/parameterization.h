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

        // edge to vertex
        // 非边缘边需要依赖边缘边的数据，因为非边缘边内存在边缘点
        // 运算结束后释放边集
        void _convert_edge_to_vertex(
            std::vector<OrderedEdge>&&,
            std::vector<OrderedEdge>&&,
            std::vector<int>&,
            std::vector<int>&
        );

        // 解方程组 L_{I, I} * f(I) = -L_{B, B} * f(B)
        // or L_{B, B} * f(B) = -L_{B, I} * f(I)
        void _solve_Laplacian_equation(
            const std::vector<int>& r_idx_1,
            const std::vector<int>& c_idx_1,
            std::vector<glm::vec2>& f_1,  // 结果保存在这里
            const std::vector<int>& r_idx_2,
            const std::vector<int>& c_idx_2,
            const std::vector<glm::vec2>& f_2
        );

    private:
        // 中间结果
        float m_bound_length; // 边缘总长度

    private:
        MeshDrawable* m_ori;
        MeshDrawable* m_tar;
    };
}

#endif
