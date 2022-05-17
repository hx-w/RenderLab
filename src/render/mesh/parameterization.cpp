#include "parameterization.h"
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <utility>
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
        set<int> vidx_bound;
        set<int> vidx_inner;
        // 先获取边缘点和非边缘点的索引
        _remark_vertices(vidx_bound, vidx_inner);

    }

    void Parameterization::_remark_vertices(std::set<int>& vidx_bound, std::set<int>& vidx_inner) {
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
                vidx_bound.insert(edge.first);
                vidx_bound.insert(edge.second);
            }
            else if (count == 2) {
                vidx_inner.insert(edge.first);
                vidx_inner.insert(edge.second);
            }
            else {
                cout << "Error: edge count is not 1 or 2" << endl;
            }
        }
    }
}
