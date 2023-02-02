#include "elements.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../executor.h"


using namespace std;

#define _REMESH_COMMAND_FORMAT string("python3 scripts/remesh/run.py --input %s --output %s")

namespace RenderSpace {
    bool MeshDrawable::load_OBJ(const std::string& filename, bool validate) {
        if (validate) {
           //  command(_REMESH_COMMAND_FORMAT + " --validate", filename.c_str(), filename.c_str());
        }
        _reset();
        _ready_to_draw = false;
        _ready_to_update = false;
        ifstream ifs(filename);
        if (!ifs.good()) {
            // Logger::log("Failed to open file: " + filename, imgui_ext::LOG_ERROR);
            return false;
        }
        m_filename = filename;

        string line;
        while (getline(ifs, line)) {
            if (line.empty()) {
                continue;
            }
            if (line[0] == '#') {
                continue;
            }
            vector<string> words;
            _split_words(line, words);
            if (words[0] == "v") {
                m_vertices.emplace_back(
                    glm::vec3(
                        stof(words[1]),
                        stof(words[2]),
                        stof(words[3])
                    ),
                    glm::vec3(0.5, 0.5, 0.5),
                    glm::vec3(0.0, 0.0, 0.0)
                );
            }
            else if (words[0] == "f") {
                int v1 = stoi(words[1]) - 1;
                int v2 = stoi(words[2]) - 1;
                int v3 = stoi(words[3]) - 1;
                m_triangles.emplace_back(Triangle(v1, v2, v3));
            }
        }
        ifs.close();
        // 计算法线
        for (int i = 0; i < m_triangles.size(); ++i) {
            Triangle& tri = m_triangles[i];
            glm::vec3 v1 = m_vertices[tri.VertexIdx.x].Position;
            glm::vec3 v2 = m_vertices[tri.VertexIdx.y].Position;
            glm::vec3 v3 = m_vertices[tri.VertexIdx.z].Position;
            glm::vec3 normal = -glm::normalize(glm::cross(v2 - v1, v3 - v1));
            m_vertices[tri.VertexIdx.x].Normal += normal;
            m_vertices[tri.VertexIdx.y].Normal += normal;
            m_vertices[tri.VertexIdx.z].Normal += normal;
            // normalize normal
            m_vertices[tri.VertexIdx.x].Normal = glm::normalize(m_vertices[tri.VertexIdx.x].Normal);
            m_vertices[tri.VertexIdx.y].Normal = glm::normalize(m_vertices[tri.VertexIdx.y].Normal);
            m_vertices[tri.VertexIdx.z].Normal = glm::normalize(m_vertices[tri.VertexIdx.z].Normal);
        }
        ready_to_update();
        return true;
    }

    bool MeshDrawable::save_OBJ(const string& filename) {
        ofstream ofs(filename);
        if (!ofs.good()) {
            // Logger::log("Failed to open file: " + filename, imgui_ext::LOG_ERROR);
            return false;
        }
        for (int i = 0; i < m_vertices.size(); ++i) {
            ofs << "v " << m_vertices[i].Position.x << " " << m_vertices[i].Position.y << " " << m_vertices[i].Position.z << endl;
        }
        for (int i = 0; i < m_triangles.size(); ++i) {
            ofs << "f " << m_triangles[i].VertexIdx.x + 1 << " " << m_triangles[i].VertexIdx.y + 1 << " " << m_triangles[i].VertexIdx.z + 1 << endl;
        }
        ofs.close();
        return true;
    }

    // 需要同步更新 center aabb radius
    void MeshDrawable::add_vertex_raw(const Vertex& v) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_vertices.emplace_back(v);
    }

    void MeshDrawable::add_triangle_by_idx(const Triangle& tri) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_triangles.emplace_back(tri);
        // 还可以计算法线
    }

    void MeshDrawable::add_triangle_raw(const Vertex& v1, const Vertex& v2, const Vertex& v3) {
        std::lock_guard<std::mutex> lk(m_mutex);
        // 查重
        m_vertices.emplace_back(v1);
        m_vertices.emplace_back(v2);
        m_vertices.emplace_back(v3);
        m_triangles.emplace_back(Triangle(m_vertices.size() - 3, m_vertices.size() - 2, m_vertices.size() - 1));
    }

    void MeshDrawable::add_edge_raw(const Vertex& v1, const Vertex& v2) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_vertices.emplace_back(v1);
        m_vertices.emplace_back(v2);
        m_edges.emplace_back(Edge(m_vertices.size() - 2, m_vertices.size() - 1));
    }

    void MeshDrawable::_split_words(const string& line, vector<string>& words, const char delim) {
        stringstream ss(line);
        string word;
        while (getline(ss, word, delim)) {
            words.emplace_back(word);
        }
    }

    // void MeshDrawable::remesh(RenderService* service) {
    //     if (_remesh_check()) {
    //         // std::thread([&]() {
    //             Logger::log("start to remesh", imgui_ext::LOG_INFO);
    //             command(
    //                 _REMESH_COMMAND_FORMAT + " --remesh --pivots %d %d %d %d",
    //                 m_filename.c_str(), (m_filename + ".remesh.obj").c_str(),
    //                 m_picked_vertices[0], m_picked_vertices[1], m_picked_vertices[2], m_picked_vertices[3]
    //             );
    //             Logger::log("remesh finished", imgui_ext::LOG_INFO);
    //             service->load_mesh("str_mesh", m_filename + ".remesh.obj");
    //         // }).detach();
    //     }

    //     // reset picked info
    //     for (auto& vi: m_picked_vertices) {
    //         m_vertices[vi].Color = glm::vec3(0.5, 0.5, 0.5);
    //     }
    //     if (m_picked_vertices.size() != 0) {
    //         m_picked_vertices.clear();
    //         ready_to_update();
    //     }
    // }

    bool MeshDrawable::_remesh_check() const {
        if (m_picked_vertices.size() != 4) {
            // Logger::log("picked vertices size != 4", imgui_ext::LOG_ERROR);
            return false;
        }
        return true;
    }
}
