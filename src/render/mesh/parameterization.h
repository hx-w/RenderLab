#ifndef PARAMETERIZATION_H
#define PARAMETERIZATION_H

#include "elements.h"
#include <vector>
#include <unordered_map>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace RenderSpace {
    struct pair_hash {
        template <class T1, class T2>
        std::size_t operator() (const std::pair<T1,T2> &p) const {
            auto h1 = std::hash<T1>{}(p.first);
            auto h2 = std::hash<T2>{}(p.second);
            return h1 ^ h2;  
        }
    };

    struct trias_hash {
        std::size_t operator() (const Triangle& tri) const {
            auto h1 = std::hash<uint32_t>{}(tri.VertexIdx.x);
            auto h2 = std::hash<uint32_t>{}(tri.VertexIdx.y);
            auto h3 = std::hash<uint32_t>{}(tri.VertexIdx.z);
            return h1 ^ h2 ^ h3;
        }
    };

    class Parameterization {
    public:
        Parameterization() = default;
        Parameterization(MeshDrawable* ori, MeshDrawable* tar);

        ~Parameterization();

        void parameterize();

    private:
        // 标记ori面中的边缘与非边缘
        void _remark_edges(std::vector<OrderedEdge>&, std::vector<OrderedEdge>&);
        // 对边缘边，从第一个边缘点开始 按拓扑关系进行重新排序
        void _topology_reorder(std::vector<OrderedEdge>&);
        // 将排序后的边缘边参数化到二维单位圆边缘
        void _parameterize_bound(std::vector<OrderedEdge>&, std::vector<glm::vec2>&);
        // 初始化 计算weights
        void _init_weights(
            const std::vector<OrderedEdge>&,
            const std::vector<OrderedEdge>&,
            std::unordered_map<OrderedEdge, float, pair_hash>&
        );

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

        // cotangent
        float _cot(float) const;
        float _angle_between(const glm::vec3&, const glm::vec3&, const glm::vec3&) const;

    private:
        // 中间结果
        float m_bound_length; // 边缘总长度

    private:
        MeshDrawable* m_ori;
        MeshDrawable* m_tar;
    };
}

#endif
