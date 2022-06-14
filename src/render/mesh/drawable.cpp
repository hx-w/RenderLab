#include "drawable.h"

#include "../libs/glad/glad.h"
#include "../libs/GLFW/glfw3.h"
#include "../imgui_ext/logger.h"
#include <iostream>

using namespace std;

namespace RenderSpace {
    Drawable::Drawable() {
        // 默认随机生成一个名字
        std::lock_guard<std::mutex> lk(m_mutex);
        m_name = "Drawable_" + to_string(rand());
        m_type = DrawableType::DRAWABLE_POINT;
        _ready_to_draw = false;
        _ready_to_update = false;
    }

    Drawable::~Drawable() {
        std::lock_guard<std::mutex> lk(m_mutex);
        _ready_to_draw = false;
        _ready_to_update = false;
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_vbo);
        glDeleteBuffers(1, &m_ebo);
        _reset();
    }

    Drawable::Drawable(const Drawable& element) {
        _deepcopy(element);
    }

    Drawable& Drawable::operator=(const Drawable& element) {
        _deepcopy(element);
        return *this;
    }

    void Drawable::set_shader(Shader& shader) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_shader = shader;
    }

    void Drawable::set_type(DrawableType type) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_type = type;
        if (type == DrawableType::DRAWABLE_POINT) {
            m_shade_mode = GL_POINT;
        }
        else if (type == DrawableType::DRAWABLE_LINE && m_shade_mode == GL_FILL) {
            m_shade_mode = GL_LINE;
        }
    }

    void Drawable::set_shade_mode(GLenum mode) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_shade_mode = mode;
    }

    void Drawable::draw() {
        std::lock_guard<std::mutex> lk(m_mutex);
        if (!_ready_to_draw) return;
        m_shader.use();
        glBindVertexArray(m_vao);

        // render elements
        glm::mat4 model = glm::mat4(1.0f);
        m_shader.setMat4("model", model);
        glPointSize(3.0f);

        // polygon mode
        glPolygonMode(GL_FRONT_AND_BACK, m_shade_mode);

        switch (m_type) {
        case DrawableType::DRAWABLE_POINT:
            m_shader.setBool("ignoreLight", true);
            glDrawArrays(GL_POINTS, 0, m_vertices.size());
            break;
        case DrawableType::DRAWABLE_LINE:
            m_shader.setBool("ignoreLight", true);
            glLineWidth(2.0f);
            if (m_ebo != 0 && m_edges.size() > 0) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
                glDrawElements(GL_LINES, m_edges.size() * 2, GL_UNSIGNED_INT, 0);
            }
            else {
                glDrawArrays(GL_LINES, 0, m_vertices.size());
            }
            glLineWidth(1.0f);
            break;
        case DrawableType::DRAWABLE_TRIANGLE:
            m_shader.setBool("ignoreLight", false);
            glLineWidth(1.0f);
            if (m_ebo != 0 && m_triangles.size() > 0) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
                glDrawElements(GL_TRIANGLES, m_triangles.size() * 3, GL_UNSIGNED_INT, 0);
            }
            else {
                glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());
            }
            break;
        default:
            m_shader.setBool("ignoreLight", false);
            break;
        }

        glBindVertexArray(0);
    }

    void Drawable::sync() {
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

        switch (m_type) {
            case DrawableType::DRAWABLE_LINE:
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_edges.size() * sizeof(Edge), &m_edges[0], GL_DYNAMIC_DRAW);
                break;
            case DrawableType::DRAWABLE_TRIANGLE:
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_triangles.size() * sizeof(Triangle), &m_triangles[0], GL_DYNAMIC_DRAW);
                break;
            default:
                break;
        }
        _ready_to_draw = true;
    }

    void Drawable::_reset() {
        vector<Triangle>().swap(m_triangles);
        vector<Edge>().swap(m_edges);
        vector<Vertex>().swap(m_vertices);
    }

    void Drawable::_gen_vao() {
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_ebo);
    }

    void Drawable::ready_to_update() {
        std::lock_guard<std::mutex> lk(m_mutex);
        if (_ready_to_update) return;
        _ready_to_update = true;
    }

    void Drawable::set_visible(bool visible) {
        if (visible) {
            ready_to_update();
        }
        else {
            std::lock_guard<std::mutex> lk(m_mutex);
            _ready_to_draw = false;
        }
    }

    bool Drawable::is_visible() const {
        // std::lock_guard<std::mutex> lk(m_mutex);
        return _ready_to_draw;
    }

    void Drawable::_deepcopy(const Drawable& element) {
        std::lock_guard<std::mutex> lk(m_mutex);
        _ready_to_draw = false;
        _ready_to_update = false;
        m_type = element.m_type;
        m_triangles.assign(element.m_triangles.begin(), element.m_triangles.end());
        m_edges.assign(element.m_edges.begin(), element.m_edges.end());
        m_vertices.assign(element.m_vertices.begin(), element.m_vertices.end());
        m_center = element.m_center;
        m_radius = element.m_radius;
        m_aabb = element.m_aabb;
    }
}
