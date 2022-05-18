#include "elements.h"
#include <iostream>

#include "../libs/glad/glad.h"
#include "../libs/GLFW/glfw3.h"

using namespace std;
namespace RenderSpace {
    bool MeshDrawable::load_STL(const std::string& filename) {
        m_mutex.lock();
        ifstream ifs(filename);
        if (!ifs.good()) {
            cout << "[ERROR] " << "Can't open file: " << filename << endl;
            return false;
        }
        string headStr;
        getline(ifs, headStr, ' ');
        ifs.close();
        m_mutex.unlock();

        if (headStr.empty()) {
            return false;
        }

        _reset();
        bool success = false;
        if (headStr[0] == 's') {
            success = _read_STL_ASCII(filename);
        }
        else {
            success =  _read_STL_Binary(filename);
        }
        if (success) {
            ready_to_update();
        }
        return success;
    }

    bool MeshDrawable::_read_STL_ASCII(const std::string& filename) {
        std::lock_guard<std::mutex> lk(m_mutex);
        ifstream ifs(filename);
        if (!ifs.good()) {
            return false;
        }

        // unimplemented
        return false;
    }

    bool MeshDrawable::_read_STL_Binary(const std::string& filename) {
        // std::lock_guard<std::mutex> lk(m_mutex);
        ifstream ifs(filename, ios::binary);
        if (!ifs.good()) {
            return false;
        }
        
        const int _int_size = sizeof(int);
        const int _float_size = sizeof(float);
        ifs.ignore(80);

        // 面的个数
        int num_trias;
        ifs.read((char*)(&num_trias), _int_size);
        // cout << "面片数量：" << num_trias << endl;

        float tn0, tn1, tn2;
        float v0, v1, v2;
        float cx = 0.0, cy = 0.0, cz = 0.0;

        for (int i = 0; i < num_trias; ++i) {
            ifs.read((char*)(&tn0), _float_size);
            ifs.read((char*)(&tn1), _float_size);
            ifs.read((char*)(&tn2), _float_size);
            
            // 01-STL model
            ifs.read((char*)(&v0), _float_size);
            ifs.read((char*)(&v1), _float_size);
            ifs.read((char*)(&v2), _float_size);

            Vertex vt1(
                glm::vec3(v0, v1, v2),
                glm::vec3(1.0, 1.0, 1.0),
                glm::vec3(tn0, tn1, tn2)
            );
            cx += v0; cy += v1; cz += v2;

            ifs.read((char*)(&v0), _float_size);
            ifs.read((char*)(&v1), _float_size);
            ifs.read((char*)(&v2), _float_size);

            Vertex vt2(
                glm::vec3(v0, v1, v2),
                glm::vec3(1.0, 1.0, 1.0),
                glm::vec3(tn0, tn1, tn2)
            );
            cx += v0; cy += v1; cz += v2;

            ifs.read((char*)(&v0), _float_size);
            ifs.read((char*)(&v1), _float_size);
            ifs.read((char*)(&v2), _float_size);

            Vertex vt3(
                glm::vec3(v0, v1, v2),
                glm::vec3(1.0, 1.0, 1.0),
                glm::vec3(tn0, tn1, tn2)
            );
            cx += v0; cy += v1; cz += v2;

            // 建立面片索引，确定顶点顺序
            add_triangle_raw(vt1, vt2, vt3);

            ifs.ignore(2);
        }
        ifs.close();
        
        // 计算中心位置
        m_center.x = cx / (num_trias * 3);
        m_center.y = cy / (num_trias * 3);
        m_center.z = cz / (num_trias * 3);

        //计算半径
        m_radius = 0;
        for (int i = 0; i < m_vertices.size(); ++i) {
            m_vertices[i].Position = m_vertices[i].Position - m_center;
            float lens = sqrt(glm::dot(m_vertices[i].Position, m_vertices[i].Position));
            if (lens > m_radius) {
                m_radius = lens;
            }
        }

        // cout << "radius: " << m_radius << " center: " << m_center.x << "," << m_center.y << "," << m_center.z << endl;
        return true;
    }

    bool MeshDrawable::load_OBJ(const std::string& filename) {
        _reset();
        _ready_to_draw = false;
        _ready_to_update = false;
        ifstream ifs(filename);
        if (!ifs.good()) {
            cout << "[ERROR] " << "Can't open file: " << filename << endl;
            return false;
        }

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
                    glm::vec3(1.0, 1.0, 1.0),
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
            glm::vec3 normal = glm::normalize(glm::cross(v2 - v1, v3 - v1));
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

    // 需要同步更新 center aabb radius
    void MeshDrawable::add_vertex_raw(const Vertex& v) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_vertices.push_back(v);
    }

    void MeshDrawable::add_triangle_by_idx(const Triangle& tri) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_triangles.push_back(tri);
        // 还可以计算法线
    }

    void MeshDrawable::add_triangle_raw(const Vertex& v1, const Vertex& v2, const Vertex& v3) {
        std::lock_guard<std::mutex> lk(m_mutex);
        // 查重
        m_vertices.push_back(v1);
        m_vertices.push_back(v2);
        m_vertices.push_back(v3);
        m_triangles.push_back(Triangle(m_vertices.size() - 3, m_vertices.size() - 2, m_vertices.size() - 1));
    }

    void MeshDrawable::add_edge_raw(const Vertex& v1, const Vertex& v2) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_vertices.push_back(v1);
        m_vertices.push_back(v2);
        m_edges.push_back(Edge(m_vertices.size() - 2, m_vertices.size() - 1));
    }

    void MeshDrawable::_split_words(const string& line, vector<string>& words, const char delim) {
        stringstream ss(line);
        string word;
        while (getline(ss, word, delim)) {
            words.push_back(word);
        }
    }
}
