#include "parameterization.h"

#include <omp.h>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <unordered_set>
#include <utility>
#include <chrono>
#include <ctime>
#include <thread>
#include "../libs/tgaimage/tgaimage.h"

using namespace std;
using namespace glm;

namespace RenderSpace {
Parameterization::Parameterization(
    shared_ptr<MeshDrawable> uns_mesh,
    shared_ptr<MeshDrawable> param_mesh,
    shared_ptr<MeshDrawable> str_mesh
    ): m_uns_mesh(uns_mesh), m_param_mesh(param_mesh), m_str_mesh(str_mesh), m_scale(10) {}

Parameterization::~Parameterization() {}

bool Parameterization::parameterize(ParamMethod pmodel, float& progress, uint32_t num_samples) {
    progress = 0.0f;
    vector<OrderedEdge> edge_bound;
    vector<OrderedEdge> edge_inner;
    // 先获取边缘和非边缘边
    _remark_edges(edge_bound, edge_inner);
    if (edge_bound.empty()) {
        return false;
        _cut_mesh_open(edge_inner);
        _remark_edges(edge_bound, edge_inner);
    }
    // 需要将边缘边有序遍历 v1->v3, v3->v2, v2->v1
    _topology_reorder(edge_bound);
    // 参数平面 边缘点 根据edge_bound 顺序计算得到
    vector<vec2> param_bound;
    _parameterize_bound(edge_bound, param_bound);
    cout << "param_bound mapping finished" << endl;
    // 初始化weights
    // weight[(i, j)] = weight[(j, i)], 故存储时令i < j
    // 这里元素量太大，可以使用map进行优化，但是map对于key的查找有问题(?)
    _init_weights(edge_bound, edge_inner, pmodel);

    // 将边集 转换为 点集 保存映射的顺序关系
    // 其中边缘点的顺序不能变，应与param_bound一致
    vector<int> vt_bound;
    vector<int> vt_inner;
    _convert_edge_to_vertex(move(edge_bound), move(edge_inner), vt_bound,
                            vt_inner);
    // 解方程组
    vector<vec2> param_inner;
    cout << "solving Laplacian equation" << endl;
    _solve_Laplacian_equation(vt_inner, vt_inner, param_inner, vt_inner,
                              vt_bound, param_bound, progress, num_samples);
    cout << "building mesh" << endl;
    _build_param_mesh(vt_inner, vt_bound, param_inner, param_bound);
    return true;
}

void Parameterization::resample(uint32_t num_samples) {
    const auto uns_vertices = m_uns_mesh->get_vertices();
    const auto param_vertices = m_param_mesh->get_vertices();
    vector<Vertex> str_vertices;
    vector<Triangle> str_trias;

    TGAImage image(int(num_samples), int(num_samples), TGAImage::RGB);
    for (auto ir = 0; ir < num_samples; ++ir) {
        for (auto ic = 0; ic < num_samples; ++ic) {
            // 在参数平面上的点
            auto x = m_scale * (ic + 0.5) / num_samples - m_scale / 2;
            auto y = m_scale * (ir + 0.5) / num_samples - m_scale / 2;
            // 逆映射到三维空间
            const Triangle spot_trias = _which_trias_in(vec2(x, y));
            auto tot_area = _trias_area(param_vertices[spot_trias.VertexIdx.x].Position,
                                        param_vertices[spot_trias.VertexIdx.y].Position,
                                        param_vertices[spot_trias.VertexIdx.z].Position);
            auto vi_area = _trias_area(vec3(x, y, 0),
                                       param_vertices[spot_trias.VertexIdx.y].Position,
                                       param_vertices[spot_trias.VertexIdx.z].Position);
            auto vj_area = _trias_area(param_vertices[spot_trias.VertexIdx.x].Position,
                                       vec3(x, y, 0),
                                       param_vertices[spot_trias.VertexIdx.z].Position);
            auto vk_area = _trias_area(param_vertices[spot_trias.VertexIdx.x].Position,
                                        param_vertices[spot_trias.VertexIdx.y].Position,
                                        vec3(x, y, 0));
            vec3 str_point = (vi_area * uns_vertices[spot_trias.VertexIdx.x].Position +
                              vj_area * uns_vertices[spot_trias.VertexIdx.y].Position +
                              vk_area * uns_vertices[spot_trias.VertexIdx.z].Position) / tot_area;
            
            str_vertices.emplace_back(Vertex(str_point, vec3(0.5), vec3(0.0)));

           for (int idx = 0; idx < 4; ++idx) {
               image.set(ic * 2 + idx % 2, ir * 2 + idx / 2, TGAColor(
                   static_cast<uint8_t>(((*reinterpret_cast<uint32_t*>(&str_point.x)) >> (idx * 8)) & 0xFF),
                   static_cast<uint8_t>(((*reinterpret_cast<uint32_t*>(&str_point.y)) >> (idx * 8)) & 0xFF),
                   static_cast<uint8_t>(((*reinterpret_cast<uint32_t*>(&str_point.z)) >> (idx * 8)) & 0xFF)
               ));
           }
            /**
             *  retopology
             *  [idx-max_col-1] ----- [idx-max_col]
             *   |            /        |
             *  [idx-1] ------------- [idx] 
             */
            if (ir > 0 && ic > 0) {
                auto idx = ir * num_samples + ic;
                str_trias.emplace_back(Triangle(idx, idx - num_samples, idx - 1));
                str_trias.emplace_back(Triangle(idx - 1, idx - num_samples, idx - num_samples - 1));
            }
        }
    }

    // 计算法线
    for (int i = 0; i < str_trias.size(); ++i) {
        Triangle& tri = str_trias[i];
        glm::vec3 v1 = str_vertices[tri.VertexIdx.x].Position;
        glm::vec3 v2 = str_vertices[tri.VertexIdx.y].Position;
        glm::vec3 v3 = str_vertices[tri.VertexIdx.z].Position;
        glm::vec3 normal = -glm::normalize(glm::cross(v2 - v1, v3 - v1));
        str_vertices[tri.VertexIdx.x].Normal += normal;
        str_vertices[tri.VertexIdx.y].Normal += normal;
        str_vertices[tri.VertexIdx.z].Normal += normal;
        // normalize normal
        str_vertices[tri.VertexIdx.x].Normal = glm::normalize(str_vertices[tri.VertexIdx.x].Normal);
        str_vertices[tri.VertexIdx.y].Normal = glm::normalize(str_vertices[tri.VertexIdx.y].Normal);
        str_vertices[tri.VertexIdx.z].Normal = glm::normalize(str_vertices[tri.VertexIdx.z].Normal);
    }
    auto& str_v = m_str_mesh->get_vertices();
    str_v.swap(str_vertices);
    auto& str_tri = m_str_mesh->get_triangles();
    str_tri.swap(str_trias);
    image.write_tga_file("static/geoimage/resample.tga");

    m_str_mesh->ready_to_update();
}

void Parameterization::_remark_edges(vector<OrderedEdge>& edge_bound,
                                     vector<OrderedEdge>& edge_inner) {
    edge_bound.clear();
    edge_inner.clear();
    set<OrderedEdge> edge_bound_set;
    set<OrderedEdge> edge_inner_set;
    // 构造Edge集合，判断只与一个三角形相邻的边
    unordered_map<OrderedEdge, int, pair_hash> edge_count_map;
    const auto& trias = m_uns_mesh->get_triangles();
    for (auto& tri : trias) {
        for (int i = 0; i < 3; ++i) {
            int vidx = std::min(tri.VertexIdx[i], tri.VertexIdx[(i + 1) % 3]);
            int vidx_next = std::max(tri.VertexIdx[i], tri.VertexIdx[(i + 1) % 3]);
            OrderedEdge edge(vidx, vidx_next);
            edge_count_map[edge]++;
        }
    }
    for (auto& [edge, count] : edge_count_map) {
        if (count == 1) {
            edge_bound_set.insert(edge);
        } else if (count == 2) {
            edge_inner_set.insert(edge);
        } else {
            cout << "Error: edge count is not 1 or 2" << endl;
        }
    }
    edge_bound.assign(edge_bound_set.begin(), edge_bound_set.end());
    edge_inner.assign(edge_inner_set.begin(), edge_inner_set.end());
}

void Parameterization::_cut_mesh_open(const vector<OrderedEdge>& tot_edge) {
    // 对于不同亏格的流形需要不同的切割方法，这里默认亏格为0
    // 随机找路径并切割
    vector<OrderedEdge> cutpath;
    // 邻接表
    unordered_map<int, unordered_set<int>> adj_vlist;
    for (auto& edge : tot_edge) {
        adj_vlist[edge.first].insert(edge.second);
        adj_vlist[edge.second].insert(edge.first);
    }
    _find_cutpath(cutpath, adj_vlist);
    _build_mesh_by_cutpath(cutpath);
}

void Parameterization::_topology_reorder(vector<OrderedEdge>& edge_bound) {
    if (edge_bound.empty())
        return;
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
    const auto& vertices = m_uns_mesh->get_vertices();
    m_bound_length = 0;

    while (visited.count(vidx) == 0) {
        visited.insert(vidx);
        int vidx_next = adj_list[vidx][0];
        if (visited.count(adj_list[vidx][0]) != 0) {
            // 判断是否首尾相接
            if (visited.count(adj_list[vidx][1]) != 0) {
                vidx_next = vidx_reserved;
            } else {
                vidx_next = adj_list[vidx][1];
            }
        }
        edge_bound_reorder.emplace_back(OrderedEdge(vidx, vidx_next));
        m_bound_length +=
            length(vertices[vidx].Position - vertices[vidx_next].Position);
        vidx = vidx_next;
    }
    cout << "topology reorder: " << edge_bound.size() << " == " << edge_bound_reorder.size() << endl;
    edge_bound.swap(edge_bound_reorder);
}

void Parameterization::_parameterize_bound(vector<OrderedEdge>& edge_bound,
                                           vector<vec2>& param_bound) {
    if (edge_bound.empty())
        return;
    // 参数平面 边缘点 根据edge_bound 顺序计算得到
    const auto& vertices = m_uns_mesh->get_vertices();
    // 三维空间中网格的边缘会被映射到二维参数平面的单位圆/或正方形边缘
    // param_x^j = sin(\theta^j)
    // param_y^j = cos(\theta^j)
    // \theta^j = 2 * \pi (\sum_{i=1}^{j} (vb_{i + 1} - vb_{i})) /
    // m_bound_length 其中 vb_{i + 1} - vb_{i} 为边缘点的距离
    param_bound.clear();
    float _accumulate_length = 0.0;
    bool disturbed_1 = false;
    bool disturbed_2 = false;
    bool disturbed_3 = false;
    for (auto& edge : edge_bound) {
        _accumulate_length +=
            length(vertices[edge.first].Position - vertices[edge.second].Position);

        // float _theta = 2 * M_PI * _accumulate_length / m_bound_length;
        // param_bound.push_back(vec2(sin(_theta) * 10, cos(_theta) * 10));
        // continue;
        
        /**
         * mapping to rectangle bound
         * make sure every corner of rect is mapped to a vertex
         */
        vec2 bound_point(0.0, 0.0);
        float ratio = _accumulate_length / m_bound_length;
        if (ratio < 0.25) {
            bound_point.x = -(m_scale / 2) + m_scale * (ratio / 0.25);
            bound_point.y = -(m_scale / 2);
        } else if (ratio < 0.5) {
            if (!disturbed_1) {
                disturbed_1 = true;
                ratio = 0.25;
            }
            bound_point.x = (m_scale / 2);
            bound_point.y = -(m_scale / 2) + m_scale * ((ratio - 0.25) / 0.25);
        } else if (ratio < 0.75) {
            if (!disturbed_2) {
                disturbed_2 = true;
                ratio = 0.5;
            }
            bound_point.x = (m_scale / 2) - m_scale * ((ratio - 0.5) / 0.25);
            bound_point.y = (m_scale / 2);
        } else {
            if (!disturbed_3) {
                disturbed_3 = true;
                ratio = 0.75;
            }
            bound_point.x = -(m_scale / 2);
            bound_point.y = (m_scale / 2) - m_scale * ((ratio - 0.75) / 0.25);
        }
        param_bound.push_back(bound_point);
    }
}

void Parameterization::_init_weights(
    const vector<OrderedEdge>& edge_bound,
    const vector<OrderedEdge>& edge_inner,
    const ParamMethod pmodel) {
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
    const auto& vertices = m_uns_mesh->get_vertices();
    const auto& triangles = m_uns_mesh->get_triangles();
    unordered_set<Triangle, trias_hash> trias_set(triangles.begin(),
                                                  triangles.end());
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
    for (auto& edge : edge_inner) {
        adj_list[edge.first].insert(edge.second);
        adj_list[edge.second].insert(edge.first);
    }
    for (auto& edge : edge_bound) {
        adj_list[edge.first].insert(edge.second);
        adj_list[edge.second].insert(edge.first);
    }

    // weights只需从inner构造
    for (auto edge : edge_inner) {
        int vi = std::min(edge.first, edge.second);
        int vj = std::max(edge.first, edge.second);
        vector<int> adj_vt;
        for (auto& adj_v : adj_list[vi]) {
            // 判断(adj_v, vj, vi)是否构成三角形
            if (adj_v != vj && trias_set.count(Triangle(vi, vj, adj_v)) != 0) {
                adj_vt.emplace_back(adj_v);
            }
        }
        if (adj_vt.size() != 2) {
            cout << "Error: edge_inner " << vi << " " << vj << ": "
                 << adj_vt.size() << endl;
            continue;
        }

        float _weight = 0.0f;

        if (pmodel == ParamMethod::Laplace) {
            for (auto vk : adj_vt) {
                _weight += _cot(_angle_between(vertices[vi].Position,
                                            vertices[vj].Position,
                                            vertices[vk].Position));
            }
            _weight /= static_cast<float>(adj_vt.size());

            m_weights[OrderedEdge(vi, vj)] = -_weight;
            // weights中 i=j无意义，但是可以预存ij相等的情况，方便Laplacian
            // matrix的计算 默认值是0
            m_weights[OrderedEdge(vi, vi)] += _weight;
            m_weights[OrderedEdge(vj, vj)] += _weight;
        }
        else if (pmodel == ParamMethod::Spring) {
            // compute \lambda_{ij} = D_{ij} / \sum_{k \in N_i} D_{ik}
            // assume D_{ij} = 1
            float sum_weight = 0.0f;
            for (auto vk : adj_list[vi]) {
                sum_weight += 1.0f; // modify if D_{ij} != 1
            }
            _weight = 1.0f / sum_weight;
            m_weights[OrderedEdge(vi, vj)] = -_weight;
            m_weights[OrderedEdge(vi, vi)] += _weight;
            m_weights[OrderedEdge(vj, vj)] += _weight;
        }
        else {
            // unknown model
            assert(false);
        }
    }
}

void Parameterization::_convert_edge_to_vertex(vector<OrderedEdge>&& edge_bound,
                                               vector<OrderedEdge>&& edge_inner,
                                               vector<int>& vt_bound,
                                               vector<int>& vt_inner) {
    if (edge_bound.empty() || edge_inner.empty()) {
        return;
    }
    vt_bound.clear();
    vt_inner.clear();
    // 计算vt_bound
    // 由于边缘边拓扑有序，所以尽量避免在vt_bound中查重操作
    const auto sz = edge_bound.size();
    // 最后一个边包含的两个点都应已经被包含在vt_bound中
    vt_bound.emplace_back(edge_bound[0].first);
    for (size_t eidx = 0; eidx < sz - 1; ++eidx) {
        const int _last_vt = vt_bound.back();
        int picked = -1;
        if (edge_bound[eidx].first == _last_vt) {
            picked = edge_bound[eidx].second;
        } else if (edge_bound[eidx].second == _last_vt) {
            picked = edge_bound[eidx].first;
        }
        if (picked == -1) {
            cout << "Error: edge_bound is not topology sorted" << endl;
            continue;
        }
        if (find(vt_bound.begin(), vt_bound.end(), picked) == vt_bound.end()) {
            vt_bound.emplace_back(picked);
        }
    }
    // 计算vt_inner
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
    vector<vec2>& f_1,  // 结果保存在这里
    const vector<int>& r_idx_2,
    const vector<int>& c_idx_2,
    const vector<vec2>& f_2,
    float& progress,
    uint32_t num_samples) {
    // 约束 c_idx_2.size() == f_2.size()
    //     r_idx_1.size() == c_idx_1.size() 即矩阵1为方阵
    // 变量初始化
    const int mat1_row_count = r_idx_1.size();
    const int mat1_col_count = c_idx_1.size();
    const int mat2_row_count = r_idx_2.size();
    const int mat2_col_count = c_idx_2.size();
    assert(mat2_col_count == f_2.size());
    // L(r1, c1) * f1 = -L(r2, c2) * f2
    // 令 _value_mat = -L(r2, c2) * f2

    vector<vec2> _value_mat;
    for (int ir = 0; ir < mat2_row_count; ++ir) {
        vec2 _row_vec(0.0, 0.0);
        for (int ic = 0; ic < mat2_col_count; ++ic) {
            float _v = _Laplacian_val(r_idx_2[ir], c_idx_2[ic]);
            _row_vec += _v * f_2[ic];
        }
        _value_mat.push_back(-_row_vec);
    }
    // 设置迭代初值
    f_1.resize(mat1_col_count, vec2(0.5f, 0.5f));

    // cache index
    m_cached_index.clear();
    m_cached_index.resize(mat1_row_count);
    for (int ir = 0; ir < mat1_row_count; ++ir) {
        for (int ic = 0; ic < mat1_col_count; ++ic) {
            if (fabs(_Laplacian_val(r_idx_1[ir], c_idx_1[ic]) - 0.0f) >= 1e-6) {
                m_cached_index[ir].emplace_back(ic);
            }
        }
    }

    // 进行迭代求解
    int itermax = 500;
    for (int epoch = 0; epoch < itermax; ++epoch) {
        progress = epoch * 1.0 / itermax;
        Jacobi_Iteration(r_idx_1, c_idx_1, f_1, _value_mat, 0.1f, 5);
        _build_param_mesh(r_idx_1, c_idx_2, f_1, f_2);
        if (epoch % 5 == 0)
            resample(num_samples);
    }
}

void Parameterization::Jacobi_Iteration(const vector<int>& r_idx,
                                              const vector<int>& c_idx,
                                              vector<vec2>& f,
                                              const vector<vec2>& b,
                                              const float epsilon,  // 允许的误差
                                              const int max_iter
) {
    const int row_max = r_idx.size();
    const int col_max = c_idx.size();
    const int f_max = f.size();
    // row_max == col_max == f_max
    assert(row_max == col_max);
    assert(row_max == f_max);

    for (int _iter_count = 0; _iter_count < max_iter; ++_iter_count) {
        float _residual = 0.0f;
        auto start = chrono::system_clock::now();
        vector<vec2> _new_f(f);
        // 对于x_{f_max}^{_iter_count}
#pragma omp parallel for reduction(+:_residual)
        for (int ir = 0; ir < f_max; ++ir) {
            vec2 _val(0.0, 0.0);
            for (auto ic : m_cached_index[ir]) {
            // for (int ic = 0; ic < f_max; ++ic) {
                if (ir == ic)
                    continue;
                float _lp = _Laplacian_val(r_idx[ir], c_idx[ic]);
                _val += _lp * f[ic];
            }
            float _iv = -1.0 / _Laplacian_val(r_idx[ir], c_idx[ir]);
            _val = (_val - b[ir]) * _iv;
            _residual += sqrt(length(f[ir] - _val));
            _new_f[ir] = _val;
        }
        f.assign(_new_f.begin(), _new_f.end());
        auto end = chrono::system_clock::now();
        chrono::duration<double> elapsed_seconds = end - start;
        time_t end_time = chrono::system_clock::to_time_t(end);
        // if (_iter_count % 50 == 0) {
        //     cout << ">> " << ctime(&end_time) << _iter_count << "/" << max_iter << " ==> " << _residual << "  | cost " << elapsed_seconds.count() << endl;
        // }
        if (_residual < epsilon) {
            break;
        }
    }
}

void Parameterization::_build_param_mesh(const vector<int>& vt_inner,
                                   const vector<int>& vt_bound,
                                   const vector<vec2>& param_inner,
                                   const vector<vec2>& param_bound) {
    // 对vt_inner, vt_bound构建倒排索引
    map<int, int> vt_inner_idx;
    map<int, int> vt_bound_idx;
    for (int i = 0; i < vt_inner.size(); ++i) {
        vt_inner_idx[vt_inner[i]] = i;
    }
    for (int i = 0; i < vt_bound.size(); ++i) {
        vt_bound_idx[vt_bound[i]] = i;
    }

    vector<Vertex> tar_vertices;
    vector<Triangle> tar_tris;
    const auto ori_vertices = m_uns_mesh->get_vertices();
    const auto ori_tris = m_uns_mesh->get_triangles();
    tar_vertices.clear();

    // 三角面片索引应相同
    tar_tris.assign(ori_tris.begin(), ori_tris.end());

    // 重设顶点位置
    const auto sz = ori_vertices.size();
    for (int i = 0; i < sz; ++i) {
        vec2 _v(0.0, 0.0);
        if (vt_inner_idx.find(i) != vt_inner_idx.end()) {
            _v = param_inner[vt_inner_idx[i]];
        } else if (vt_bound_idx.find(i) != vt_bound_idx.end()) {
            _v = param_bound[vt_bound_idx[i]];
        } else {
            cout << "[ERROR] 发现非法顶点索引" << endl;
            continue;
        }
        tar_vertices.emplace_back(Vertex(vec3(_v.x, _v.y, 0.0), vec3(0.0), vec3(1.0)));
    }

    auto& param_vt = m_param_mesh->get_vertices();
    auto& param_tris = m_param_mesh->get_triangles();
    param_vt.swap(tar_vertices);
    param_tris.swap(tar_tris);
    m_param_mesh->ready_to_update();
}

float Parameterization::_Laplacian_val(int i, int j) {
    if (i > j) swap(i, j);
    auto iter = m_weights.find(OrderedEdge(i, j));
    if (iter != m_weights.end()) {
        return iter->second;
    }
    return 0.0f;
}

float Parameterization::_cot(float rad) const {
    return cos(rad) / sin(rad);
}

float Parameterization::_angle_between(const vec3& va,
                                       const vec3& vb,
                                       const vec3& ori) const {
    // calculate angle between va-ori and vb-ori by glm
    vec3 _va = normalize(va - ori);
    vec3 _vb = normalize(vb - ori);
    float _cos = dot(_va, _vb);
    float _rad = acos(_cos);
    return _rad;
}

float Parameterization::_trias_area(const vec3& v0, const vec3& v1, const vec3& v2) const {
    float _a = length(v1 - v0);
    float _b = length(v2 - v0);
    float _c = length(v2 - v1);
    float _s = (_a + _b + _c) / 2.0f;
    return sqrt(_s * (_s - _a) * (_s - _b) * (_s - _c));
}

const Triangle Parameterization::_which_trias_in(const vec2& pos) const {
    const auto& _vertices = m_param_mesh->get_vertices();
    const auto& _tris = m_param_mesh->get_triangles();
    const auto _sz = _vertices.size();
    for (const auto& tri : _tris) {
        const vec3& _v0 = _vertices[tri.VertexIdx.x].Position;
        const vec3& _v1 = _vertices[tri.VertexIdx.y].Position;
        const vec3& _v2 = _vertices[tri.VertexIdx.z].Position;
        // judge whether pos in triangle [_v0, _v1, _v2]
        auto s = (_v0.x - _v2.x) * (pos.y - _v2.y) - (_v0.y - _v2.y) * (pos.x - _v2.x);
        auto t = (_v1.x - _v0.x) * (pos.y - _v0.y) - (_v1.y - _v0.y) * (pos.x - _v0.x);

        if ((s < 0) != (t < 0) && s != 0 && t != 0) {
            continue;
        }

        auto d = (_v2.x - _v1.x) * (pos.y - _v1.y) - (_v2.y - _v1.y) * (pos.x - _v1.x);
        if (d == 0 || (d < 0) == (s + t <= 0)) {
            return tri;
        }
    }
    return Triangle(0.0, 0.0, 0.0);
}

void Parameterization::_find_cutpath(
    vector<OrderedEdge>& cutpath,
    unordered_map<int, unordered_set<int>>& adj_vlist,
    int curr_vt,
    int depth
) {
    // dfs
    if (depth == 0) {
        return;
    }
    auto& _vlist = adj_vlist[curr_vt];
    int picked_vt = 0;
    // remove duplicated
    if (cutpath.empty()) {
        picked_vt = *_vlist.begin();
    }
    else {
        for (auto& vt : _vlist) {
            picked_vt = vt;
            for (auto& eg : cutpath) {
                if (vt == eg.first || vt == eg.second) {
                    picked_vt = -1;
                    break;
                }
            }
            if (picked_vt > 0) {
                break;
            }
        }
    }
    // shuffle(_vlist.begin(), _vlist.end(), depth ^ curr_vt);
    cutpath.emplace_back(OrderedEdge(curr_vt, picked_vt));
    return _find_cutpath(cutpath, adj_vlist, picked_vt, depth - 1);
}

void Parameterization::_build_mesh_by_cutpath(const vector<OrderedEdge>& cutpath) {
    auto& verts = m_uns_mesh->get_vertices();
    auto& tris = m_uns_mesh->get_triangles();
    auto tris_size = tris.size();
    auto cut_size = cutpath.size();
    unordered_map<int, vector<int>> adj_trias; // v => vec<trias>
    for (auto tridx = 0; tridx < tris_size; ++tridx) {
        adj_trias[tris[tridx].VertexIdx.x].emplace_back(tridx);
        adj_trias[tris[tridx].VertexIdx.y].emplace_back(tridx);
        adj_trias[tris[tridx].VertexIdx.z].emplace_back(tridx);
    }

    auto is_edge_in_triangle = [&](const OrderedEdge& edge, int tridx)-> bool {
        return ((tris[tridx].vt_in(edge.first) >= 0) && tris[tridx].vt_in(edge.second) >= 0);
    };

    int pivot_tri = -1;
    int target_vt = cutpath[0].first;
    int last_new = -1;
    if (target_vt != cutpath[1].first && target_vt != cutpath[1].second) {
        target_vt = cutpath[0].second;
    }
    // find pivot in first
    for (auto tridx = 0; tridx < tris_size; ++tridx) {
        if (is_edge_in_triangle(cutpath[0], tridx)) {
            pivot_tri = tridx;
            break;
        }
    }

    // for others
    for (auto edge_idx = 1; edge_idx < cut_size; ++edge_idx) {
        auto& edge = cutpath[edge_idx];
        assert(edge.first == target_vt || edge.second == target_vt);
        last_new = verts.size();
        verts.emplace_back(verts[target_vt]);
        // find new pivot_tri
        while (!is_edge_in_triangle(edge, pivot_tri)) {
            int new_pivot_tri = -1;
            for (auto _tri : adj_trias[target_vt]) {
                if (is_edge_in_triangle(cutpath[edge_idx - 1], _tri)) {
                    continue;
                }
                if (_tri != pivot_tri && tris[_tri].is_neighbor(tris[pivot_tri])) {
                    new_pivot_tri = _tri;
                    break;
                }
            }
            // split
            int fd_vt_idx = tris[pivot_tri].vt_in(target_vt);
            tris[pivot_tri].VertexIdx[fd_vt_idx] = last_new;
            pivot_tri = new_pivot_tri;
        }
        // update target_vt
        if (edge_idx != cut_size - 1) {
            if (edge.first == target_vt) {
                target_vt = edge.second;
            }
            else {
                target_vt = edge.first;
            }
        }
    }
    // for last one
    int fd_vt_idx = tris[pivot_tri].vt_in(target_vt);
    tris[pivot_tri].VertexIdx[fd_vt_idx] = last_new;
}

}  // namespace RenderSpace
