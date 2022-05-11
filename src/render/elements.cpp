#include "elements.h"
#include <string>
#include <iostream>

#include "./libs/glad/glad.h"
#include "./libs/GLFW/glfw3.h"

using namespace std;
namespace RenderSpace {
    MeshDrawable::MeshDrawable() {
        std::lock_guard<std::mutex> lk(m_mutex);
        // _gen_vao();
    }

    bool MeshDrawable::load_STL(const std::string& filename) {
        m_mutex.lock();
        ifstream ifs(filename);
        if (!ifs.good()) {
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
            sync();
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
        std::lock_guard<std::mutex> lk(m_mutex);
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
        cout << "面片数量：" << num_trias << endl;

        float tn0, tn1, tn2;
        float v0, v1, v2;
        float cx = 0.0, cy = 0.0, cz = 0.0;

        for (int i = 0; i < num_trias; ++i) {
            ifs.read((char*)(&tn0), _float_size);
            ifs.read((char*)(&tn1), _float_size);
            ifs.read((char*)(&tn2), _float_size);
            //如果模型进行坐标变换，需要重新计算法向量
            // faceNrm.push_back(Normal(tn0, tn1, -tn2)); 
            
            // 01-STL model
            ifs.read((char*)(&v0), _float_size);
            ifs.read((char*)(&v1), _float_size);
            ifs.read((char*)(&v2), _float_size);

            m_vertices.emplace_back(Vertex(
                glm::vec3(v0, v1, v2),
                glm::vec3(1.0, 1.0, 1.0),
                glm::vec3(tn0, tn1, tn2)
            ));
            cx += v0; cy += v1; cz += v2;

            ifs.read((char*)(&v0), _float_size);
            ifs.read((char*)(&v1), _float_size);
            ifs.read((char*)(&v2), _float_size);

            m_vertices.emplace_back(Vertex(
                glm::vec3(v0, v1, v2),
                glm::vec3(1.0, 1.0, 1.0),
                glm::vec3(tn0, tn1, tn2)
            ));
            cx += v0; cy += v1; cz += v2;

            ifs.read((char*)(&v0), _float_size);
            ifs.read((char*)(&v1), _float_size);
            ifs.read((char*)(&v2), _float_size);

            m_vertices.emplace_back(Vertex(
                glm::vec3(v0, v1, v2),
                glm::vec3(1.0, 1.0, 1.0),
                glm::vec3(tn0, tn1, tn2)
            ));
            cx += v0; cy += v1; cz += v2;

            // 建立面片索引，确定顶点顺序
            m_triangles.emplace_back(Triangle(i * 3 + 0, i * 3 + 1, i * 3 + 2));

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

        cout << "radius: " << m_radius << " center: " << m_center.x << "," << m_center.y << "," << m_center.z << endl;
        return true;
    }

    // 基础绘制
    void MeshDrawable::draw() {
        if (!_ready_to_draw) return;
        std::lock_guard<std::mutex> lk(m_mutex);
        m_shader.use();
        glBindVertexArray(m_vao);

        // render elements
        glm::mat4 model = glm::mat4(1.0f);
        m_shader.setMat4("model", model);
        glPointSize(3.0f);

        if (m_ebo != 0) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
            glDrawElements(GL_TRIANGLES, m_triangles.size() * 3, GL_UNSIGNED_INT, 0);
        }
        else {
            glDrawArrays(GL_POINTS, 0, m_vertices.size());
        }

        glBindVertexArray(0);
    }

    void MeshDrawable::ready_to_update() {
        std::lock_guard<std::mutex> lk(m_mutex);
        _ready_to_update = true;
    }

    void MeshDrawable::sync() {
        std::lock_guard<std::mutex> lk(m_mutex);
        if (!_ready_to_update) return;
        _ready_to_update = false;
        if (m_vao == 0) {
            _gen_vao();
        }
        glBindVertexArray(m_vao);

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_DYNAMIC_DRAW);
        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);
        // color
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Color));
        glEnableVertexAttribArray(1);
        // normal
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_triangles.size() * sizeof(Triangle), &m_triangles[0], GL_DYNAMIC_DRAW);
        _ready_to_draw = true;
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
        m_vertices.push_back(v1);
        m_vertices.push_back(v2);
        m_vertices.push_back(v3);
        m_triangles.push_back(Triangle(m_vertices.size() - 3, m_vertices.size() - 2, m_vertices.size() - 1));
    }
}
