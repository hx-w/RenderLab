#include "parameterization.h"
#include <iostream>
#include <map>
using namespace std;

namespace RenderSpace {
    Parameterization::Parameterization(MeshDrawable* ori, MeshDrawable* tar):
        m_ori(ori), m_tar(tar) {}

    Parameterization::~Parameterization() {}

    void Parameterization::parameterize() {
        vector<int> vidx_bound;
        vector<int> vidx_inner;
        _remark_vertices(vidx_bound, vidx_inner);
    }

    void Parameterization::_remark_vertices(std::vector<int>& vidx_bound, std::vector<int>& vidx_inner) {
        // 构造Edge集合，判断只与一个三角形相邻的边
        map<Edge, int> edge_count;
        auto func = [&edge_count](const Edge& edge) {
            if (edge_count.find(edge) == edge_count.end()) {
                edge_count[edge] = 1;
            } else {
                edge_count[edge] += 1;
            }
        };
        auto& ori_vertices = m_ori->get_vertices();
        auto& ori_triangles = m_ori->get_triangles();
        for (auto& tri : ori_triangles) {
            func(Edge(ori_vertices[tri.VertexIdx.x].Position, ori_vertices[tri.VertexIdx.y].Position, tri.VertexIdx.x, tri.VertexIdx.y));
            func(Edge(ori_vertices[tri.VertexIdx.y].Position, ori_vertices[tri.VertexIdx.z].Position, tri.VertexIdx.y, tri.VertexIdx.z));
            func(Edge(ori_vertices[tri.VertexIdx.z].Position, ori_vertices[tri.VertexIdx.x].Position, tri.VertexIdx.z, tri.VertexIdx.x));
        }
        vidx_bound.clear();
        vidx_inner.clear();
        
        map<int, int> counter;
        for (auto& [edge, count] : edge_count) {
            if (count == 1) {
                ori_vertices[edge.v0idx].Color = glm::vec3(1.0, 0.0, 0.0);
                ori_vertices[edge.v1idx].Color = glm::vec3(1.0, 0.0, 0.0);
            }
            else if (count == 2) {
                ori_vertices[edge.v0idx].Color = glm::vec3(0.0, 1.0, 0.0);
                ori_vertices[edge.v1idx].Color = glm::vec3(0.0, 1.0, 0.0);
            }
            else if (count >= 10) {
                ori_vertices[edge.v0idx].Color = glm::vec3(0.0, 0.0, 1.0);
                ori_vertices[edge.v1idx].Color = glm::vec3(0.0, 0.0, 1.0);
            }
            if (counter.find(count) == counter.end()) {
                counter[count] = 1;
            }
            else {
                counter[count] += 1;
            }
        }

        for (auto& [num, count] : counter) {
            cout << "边度：" << num << "  个数: " << count << endl;
        }
    }
}
