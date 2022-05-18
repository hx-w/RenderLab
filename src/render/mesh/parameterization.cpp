﻿#include "parameterization.h"
#include <set>
#include <map>
#include <iostream>
#include <unordered_set>
#include <algorithm>
#include <utility>

#include "../../tooth/printer.h"
using namespace std;

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
        ToothSpace::Printer::info("param_bound mapping finished");
        // 初始化weights
        // weight[(i, j)] = weight[(j, i)], 故存储时令i < j
        // 这里元素量太大，可以使用map进行优化，但是map对于key的查找有问题(?)
        _init_weights(edge_bound, edge_inner);
        // 将边集 转换为 点集 保存映射的顺序关系
        // 其中边缘点的顺序不能变，应与param_bound一致
        vector<int> vt_bound;
        vector<int> vt_inner;
        _convert_edge_to_vertex(move(edge_bound), move(edge_inner), vt_bound, vt_inner);
        // 解方程组
        vector<glm::vec2> param_inner;
        ToothSpace::Printer::info("solving Laplacian equation");
        _solve_Laplacian_equation(vt_inner, vt_inner, param_inner, vt_bound, vt_bound, param_bound);

        // 将边缘点赋值到m_tar
        m_tar->set_type(DrawableType::DRAWABLE_LINE); // 设置为线段
        size_t sz = param_bound.size();
        for (size_t idx = 0; idx < sz; ++idx) {
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
        int vidx_reserved = vidx;
        set<int> visited;

        // 顺带计算边缘总长度
        const auto& vertices = m_ori->get_vertices();
        m_bound_length = 0;

        while (visited.count(vidx) == 0) {
            visited.insert(vidx);
            int vidx_next = adj_list[vidx][0];
            if (visited.count(adj_list[vidx][0]) != 0) {
                // 判断是否首尾相接
                if (visited.count(adj_list[vidx][1]) != 0) {
                    vidx_next = vidx_reserved;
                }
                else {
                    vidx_next = adj_list[vidx][1];
                }
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

    void Parameterization::_init_weights(
        const vector<OrderedEdge>& edge_bound,
        const vector<OrderedEdge>& edge_inner
    ) {
        /**
         * weights 的计算满足以下约束
         * 1. weights的key记作(vi, vj)，满足vi <= vj
         * 2. vi与vj不会都是边缘点
         * 
         * weights[(vi, vj)] 的计算方法如下
         * 存在以边(vi, vj)为公共边的两个三角形△(vi, vl, vj)，△(vj, vk, vi)
         * 定义角度\alpha(vi, vj) = angle(vec(vi, vl), vec(vl, vj))
         *        \alpha(vj, vi) = angle(vec(vj, vk), vec(vk, vi))
         * weights[(vi, vj)] = [cot(\alpha(vi, vj)) + cot(\alpha(vj, vi))] / 2
         */
        const auto& vertices = m_ori->get_vertices();
        const auto& triangles = m_ori->get_triangles();
        unordered_set<Triangle, trias_hash> trias_set(triangles.begin(), triangles.end());
        /** 存在这样一个事实：
         *  vi的邻接点中有vk, vk的邻接点中有vj，可能并不存在三角形(vi, vj, vk)
         * (vi) --------------(vk)
         *  | \           /  /
         *  |  \        /  /
         *  |    -(vl)-  /
         *  |     /    / 
         *  |   /    /
         *  | /   /
         * (vj)-
         */
        // 构造邻接表 快速索引
        unordered_map<int, set<int>> adj_list;
        for (auto& edge : edge_bound) {
            adj_list[edge.first].insert(edge.second);
            adj_list[edge.second].insert(edge.first);
        }
        for (auto& edge : edge_inner) {
            adj_list[edge.first].insert(edge.second);
            adj_list[edge.second].insert(edge.first);
        }

        // weights只需从inner构造
        for (auto& edge : edge_inner) {
            int vi = edge.first;
            int vj = edge.second;
            vector<int> adj_vt;
            for (auto& adj_v : adj_list[vi]) {
                // 判断(adj_v, vj, vi)是否构成三角形
                if (adj_v != vj && trias_set.count(Triangle(vi, vj, adj_v)) != 0) {
                    adj_vt.push_back(adj_v);
                }
            }
            if (adj_vt.size() != 2) {
                cout << "Error: edge_inner " << vi << " " << vj << endl;
                continue;
            }
            int vk = adj_vt[0];
            int vl = adj_vt[1];
            float cot_ij = _cot(_angle_between(vertices[vi].Position, vertices[vj].Position, vertices[vl].Position));
            float cot_ji = _cot(_angle_between(vertices[vi].Position, vertices[vj].Position, vertices[vk].Position));
            float _weight = (cot_ij + cot_ji) / 2;
            m_weights[OrderedEdge(vi, vj)] = _weight;
            // weights中 i=j无意义，但是可以预存ij相等的情况，方便Laplacian matrix的计算
            // 默认值是0
            m_weights[OrderedEdge(vi, vi)] += _weight;
            m_weights[OrderedEdge(vj, vj)] += _weight;
        }
    }

    void Parameterization::_convert_edge_to_vertex(
        vector<OrderedEdge>&& edge_bound,
        vector<OrderedEdge>&& edge_inner,
        vector<int>& vt_bound,
        vector<int>& vt_inner
    ) {
        vt_bound.clear();
        vt_inner.clear();
        // 计算vt_bound
        // 由于边缘边拓扑有序，所以尽量避免在vt_bound中查重操作
        const auto sz = edge_bound.size();
        // 最后一个边包含的两个点都应已经被包含在vt_bound中
        vt_bound.emplace_back(edge_bound[0].first);
        for (size_t eidx = 0; eidx < sz - 1; ++eidx) {
            const int _last_vt = vt_bound.back();
            if (edge_bound[eidx].first == _last_vt) {
                vt_bound.emplace_back(edge_bound[eidx].second);
            }
            else if (edge_bound[eidx].second == _last_vt) {
                vt_bound.emplace_back(edge_bound[eidx].first);
            }
            else {
                cout << "Error: edge_bound is not topology sorted" << endl;
            }
        }
        // 计算vt_inner
        // 先将vt_bound 构造成集合，查找效率高 (vt_bound元素少，使用基于hash的unordered_set)
        unordered_set<int> vt_bound_set(vt_bound.begin(), vt_bound.end());
        set<int> vt_inner_set;
        for (auto& edge : edge_inner) {
            if (vt_bound_set.count(edge.first) == 0) {
                vt_inner_set.insert(edge.first);
            }
            if (vt_bound_set.count(edge.second) == 0) {
                vt_inner_set.insert(edge.second);
            }
        }

        vt_inner.assign(vt_inner_set.begin(), vt_inner_set.end());
        // 释放内存
        vector<OrderedEdge>().swap(edge_bound);
        vector<OrderedEdge>().swap(edge_inner);
    }

    void Parameterization::_solve_Laplacian_equation(
        const vector<int>& r_idx_1,
        const vector<int>& c_idx_1,
        vector<glm::vec2>& f_1,  // 结果保存在这里
        const vector<int>& r_idx_2,
        const vector<int>& c_idx_2,
        const vector<glm::vec2>& f_2
    ) {
        
    }

    float Parameterization::_Laplacian_val(int i, int j) {
        return (i == j ? 1 : -1) * m_weights[OrderedEdge(i, j)];
    }

    float Parameterization::_cot(float rad) const {
        return 1.0f / tan(rad);
    }

    float Parameterization::_angle_between(
        const glm::vec3& va, const glm::vec3& vb, const glm::vec3& ori
    ) const {
        glm::vec3 da = glm::normalize(va-ori);
        glm::vec3 db = glm::normalize(vb-ori);
        return glm::acos(glm::dot(da, db));
    }
}
