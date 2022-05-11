#include "elements.h"
#include <string>
#include <iostream>

#include "./libs/glad/glad.h"
#include "./libs/GLFW/glfw3.h"

using namespace std;
namespace RenderSpace {
    void RenderVertices::add_vertex(const Point& pnt, const Point& clr) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_vertices.emplace_back(Vertex{
            glm::vec3(pnt.x(), pnt.y(), pnt.z()),
            glm::vec3(clr.x(), clr.y(), clr.z())
        });
        m_vertex_count ++;
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

            m_vertices.emplace_back(Vertex{glm::vec3(v0, v1, v2)});
            cx += v0; cy += v1; cz += v2;

            ifs.read((char*)(&v0), _float_size);
            ifs.read((char*)(&v1), _float_size);
            ifs.read((char*)(&v2), _float_size);

            m_vertices.emplace_back(Vertex{glm::vec3(v0, v1, v2)});
            cx += v0; cy += v1; cz += v2;

            ifs.read((char*)(&v0), _float_size);
            ifs.read((char*)(&v1), _float_size);
            ifs.read((char*)(&v2), _float_size);

            m_vertices.emplace_back(Vertex{glm::vec3(v0, v1, v2)});
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
            m_vertices[i] = m_vertices[i].Position - m_center;
            float lens = sqrt(glm::dot(m_vertices[i].Position, m_vertices[i].Position));
            if (lens > m_radius) {
                m_radius = lens;
            }
        }

        cout << m_vertices.back().Position.x << " " << m_vertices.back().Position.y << " " << m_vertices.back().Position.z << endl;
        cout << "radius: " << m_radius << " center: " << m_center.x << "," << m_center.y << "," << m_center.z << endl;
        return true;
    }

    // 基础绘制
    void MeshDrawable::draw() {
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

    void MeshDrawable::sync() {
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

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_triangles.size() * sizeof(Triangle), &m_triangles[0], GL_DYNAMIC_DRAW);
    }
}
