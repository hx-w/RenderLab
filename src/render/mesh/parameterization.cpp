#include "parameterization.h"
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <utility>
#include <set>
using namespace std;

struct pair_hash {
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1,T2> &p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ h2;  
    }
};

namespace RenderSpace {
    Parameterization::Parameterization(MeshDrawable* ori, MeshDrawable* tar):
        m_ori(ori), m_tar(tar) {}

    Parameterization::~Parameterization() {}

    void Parameterization::parameterize() {
        vector<OrderedEdge> edge_bound;
        vector<OrderedEdge> edge_inner;
        // 先获取边缘和非边缘边
        _remark_edges(edge_bound, edge_inner);
        // 需要将边缘边有序遍历 v1->v3, v3->v2, v2->v1
        _topology_reorder(edge_bound);
        // 参数平面 边缘点 根据edge_bound 顺序计算得到
        vector<glm::vec2> param_bound;
        _parameterize_bound(edge_bound, param_bound);

        // 将边缘点赋值到m_tar
        m_tar->set_type(DrawableType::DRAWABLE_LINE); // 设置为线段
        size_t sz = param_bound.size();
        cout << "sz: " << sz << endl;
        for (size_t idx = 0; idx < sz; ++idx) {
            if (idx < 10) {
                cout << "param_bound[" << idx << "]: " << param_bound[idx].x << ", " << param_bound[idx].y << endl;
            }
            glm::vec3 pos = glm::vec3(param_bound[idx].x, param_bound[idx].y, 0.0f);
            glm::vec3 pos_next = glm::vec3(param_bound[(idx + 1) % sz].x, param_bound[(idx + 1) % sz].y, 0.0f);
            m_tar->add_edge_raw(
                Vertex(pos, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f)),
                Vertex(pos_next, glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f))
            );
        }
        m_tar->ready_to_update();
    }

    void Parameterization::_remark_edges(vector<OrderedEdge>& edge_bound, vector<OrderedEdge>& edge_inner) {
        set<OrderedEdge> edge_bound_set;
        set<OrderedEdge> edge_inner_set;
        // 构造Edge集合，判断只与一个三角形相邻的边
        unordered_map<OrderedEdge, int, pair_hash> edge_count_map;
        const auto& trias = m_ori->get_triangles();
        for (auto& tri : trias) {
            for (int i = 0; i < 3; ++i) {
                int vidx = min(tri.VertexIdx[i], tri.VertexIdx[(i + 1) % 3]);
                int vidx_next = max(tri.VertexIdx[i], tri.VertexIdx[(i + 1) % 3]);
                OrderedEdge edge(vidx, vidx_next);
                if (edge_count_map.find(edge) == edge_count_map.end()) {
                    edge_count_map[edge] = 1;
                } else {
                    edge_count_map[edge]++;
                }
            }
        }
        for (auto& [edge, count] : edge_count_map) {
            if (count == 1) {
                edge_bound_set.insert(edge);
            }
            else if (count == 2) {
                edge_inner_set.insert(edge);
            }
            else {
                cout << "Error: edge count is not 1 or 2" << endl;
            }
        }
        edge_bound.assign(edge_bound_set.begin(), edge_bound_set.end());
        edge_inner.assign(edge_inner_set.begin(), edge_inner_set.end());
    }

    void Parameterization::_topology_reorder(vector<OrderedEdge>& edge_bound) {
        // 对边缘边进行 拓扑关系排序
        // 先构造一个邻接表
        unordered_map<int, vector<int>> adj_list;
        for (auto& edge : edge_bound) {
            adj_list[edge.first].push_back(edge.second);
            adj_list[edge.second].push_back(edge.first);
        }
        // 理论上， 所有边缘点构成一个环，所有adj个数都为2
        vector<OrderedEdge> edge_bound_reorder;
        // 从任一点触发，构造一个拓扑序列
        int vidx = edge_bound[0].first;
        set<int> visited;

        // 顺带计算边缘总长度
        const auto& vertices = m_ori->get_vertices();
        m_bound_length = 0;

        while (visited.count(vidx) == 0) {
            visited.insert(vidx);
            int vidx_next = adj_list[vidx][0];
            if (visited.count(adj_list[vidx][0]) != 0) {
                vidx_next = adj_list[vidx][1];
            }
            edge_bound_reorder.push_back(OrderedEdge(vidx, vidx_next));

            m_bound_length += glm::distance(vertices[vidx].Position, vertices[vidx_next].Position);

            vidx = vidx_next;
        }
        edge_bound.swap(edge_bound_reorder);
    }

    void Parameterization::_parameterize_bound(vector<OrderedEdge>& edge_bound, vector<glm::vec2>& param_bound) {
        // 参数平面 边缘点 根据edge_bound 顺序计算得到
        const auto& vertices = m_ori->get_vertices();
        // 三维空间中网格的边缘会被映射到二维参数平面的单位圆上
        // param_x^j = sin(\theta^j)
        // param_y^j = cos(\theta^j)
        // \theta^j = 2 * \pi (\sum_{i=1}^{j} (vb_{i + 1} - vb_{i})) / m_bound_length
        // 其中 vb_{i + 1} - vb_{i} 为边缘点的距离
        param_bound.clear();
        float _accumulate_length = 0.0;
        for (auto& edge : edge_bound) {
            _accumulate_length += glm::distance(vertices[edge.first].Position, vertices[edge.second].Position);
            float _theta = 2 * M_PI * _accumulate_length / m_bound_length;
            param_bound.push_back(glm::vec2(sin(_theta), cos(_theta)));
        }
    }
}
