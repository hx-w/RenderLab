#include "drawable.h"
#include "../shader.h"

#include <mesh.h>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <line.hpp>
#include <cmath>

#ifndef __PI__
#define __PI__ 3.1415926535f
#endif

using namespace std;
using namespace geometry;

namespace RenderSpace {
    DrawableBase::~DrawableBase() {
        std::lock_guard<std::mutex> lk(m_mutex);
        _ready_to_draw = false;
        _ready_to_update = false;
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_vbo);
        glDeleteBuffers(1, &m_ebo);
    }

    void DrawableBase::_init_buffer() {
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
        glGenBuffers(1, &m_ebo);

        m_shade_mode = GL_FILL;
    }

    void DrawableBase::draw() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_visible || !_ready_to_draw) {
            return;
        }
        m_shader->use();
        glBindVertexArray(m_vao);

		glPointSize(2.0f);

        // shader configure
        ///1. local transform = identity
        m_shader->setMat4("model", m_model_transf);
        ///2. shade mode
        glPolygonMode(GL_FRONT_AND_BACK, m_shade_mode);
        ///3. randomColor
        m_shader->setBool("randomColor", false);

        _draw(); // custom draw

        glBindVertexArray(0);
    }

    void DrawableBase::update() {
        std::lock_guard<std::mutex> lk(m_mutex);
        if (!_ready_to_update) return;
        _ready_to_update = false;
        if (m_vao == 0) {
            _init_buffer();
        }

        _update();

        _ready_to_draw = true;
    }

    void DrawableBase::get_ready() {
        std::lock_guard<std::mutex> lk(m_mutex);
        _ready_to_update = true;
    }

    NewMeshDrawable::NewMeshDrawable(geometry::Mesh& mesh, geometry::Vector3f clr) {
        _init_buffer();
        auto vsize = mesh.get_vertices().size();
        m_vertices.resize(vsize);
        for (size_t i = 0; i < vsize; ++i) {
            m_vertices[i].Position = mesh.vertices()[i];
            m_vertices[i].Color = clr;
        }

        // calculate normal
        auto fsize = mesh.get_faces().size();
        for (size_t i = 0; i < fsize; ++i) {
            auto& face = mesh.faces()[i];
            auto& v0 = mesh.vertices()[face[0]];
            auto& v1 = mesh.vertices()[face[1]];
            auto& v2 = mesh.vertices()[face[2]];
            auto nml = glm::normalize(glm::cross(v1 - v0, v2 - v0));
            m_vertices[face[0]].Normal += nml;
            m_vertices[face[1]].Normal += nml;
            m_vertices[face[2]].Normal += nml;
        }

        for (size_t i = 0; i < m_vertices.size(); ++i) {
            m_vertices[i].Normal = glm::normalize(m_vertices[i].Normal);
        }
        m_faces = mesh.get_faces();
        m_type = GeomType::GeomTypeMesh;

        m_raw = std::make_shared<geometry::Mesh>(mesh);
    }

    void NewMeshDrawable::_draw() {
        m_shader->setBool("ignoreLight", false);
        glLineWidth(1.0f);
        if (m_ebo != 0 && !m_faces.empty()) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
            glDrawElements(GL_TRIANGLES, m_faces.size() * 3, GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());
        }
    }

    void NewMeshDrawable::_update() {
        if (m_vertices.empty()) return;
        const auto vpsize = sizeof(VertexPrimitive);

        glBindVertexArray(m_vao);

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * vpsize, &m_vertices[0], GL_STATIC_DRAW);
        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vpsize, (void*)0);
        glEnableVertexAttribArray(0);
        // color
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vpsize, (void*)offsetof(VertexPrimitive, Color));
        glEnableVertexAttribArray(1);
        // normal
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, vpsize, (void*)offsetof(VertexPrimitive, Normal));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_faces.size() * sizeof(geometry::Vector3u), &m_faces[0], GL_STATIC_DRAW);
    }

    ArrowDrawable::ArrowDrawable(Ray& ray, float length, Vector3f clr) {
        auto dir = glm::normalize(ray.get_direction());
        auto ori = ray.get_origin();

        // https://blog.sina.com.cn/s/blog_6496e38e0102vi7e.html

        // head of the length: 20%
        auto circle_center = ori + length * 0.8f;
        auto ijk = { Vector3f(1.0f, 0.0f, 0.0f), Vector3f(0.0f, 1.0f, 0.0f), Vector3f(0.0f, 0.0f, 1.0f) };

        auto a = Vector3f(0.0f), b = Vector3f(0.0f);
        // find a valid a, b
        for (auto& t : ijk) {
            auto _a = glm::cross(dir, t);
            if (glm::length(_a) >= 1e-6) {
				a = glm::normalize(_a);

				b = glm::normalize(glm::cross(dir, a));
				break;
            }
        }

        // param function
        auto circle_radius = tanf(30.0f * __PI__ / 180.0f) * (length * 0.2f);

        auto angles = { 0.0f, __PI__ / 2.0f, __PI__, __PI__ * 3.0f / 2.0f };
        
        m_vertices.clear();
        m_vertices.emplace_back(VertexPrimitive(ori, clr, Vector3f(1.0)));
        m_vertices.emplace_back(VertexPrimitive(ori + length, clr, Vector3f(1.0)));

        for (auto& ang : angles) {
            auto v = Vector3f(
                circle_center.x + circle_radius * cosf(ang) * a.x + circle_radius * sinf(ang) * b.x,
                circle_center.y + circle_radius * cosf(ang) * a.y + circle_radius * sinf(ang) * b.y,
                circle_center.z + circle_radius * cosf(ang) * a.z + circle_radius * sinf(ang) * b.z
            );
            m_vertices.emplace_back(VertexPrimitive(v, clr, Vector3f(0.0)));
        }

        m_lines.swap(vector<unsigned int>{0, 1, 2, 1, 3, 1, 4, 1, 5, 1});
        
        m_type = GeomTypeArrow;
        m_raw = std::make_shared<geometry::Ray>(ray);
    }

    void ArrowDrawable::_update() {
        if (m_vertices.empty()) return;
        const auto vpsize = sizeof(VertexPrimitive);

        glBindVertexArray(m_vao);

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * vpsize, m_vertices.data(), GL_STATIC_DRAW);
        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vpsize, (void*)0);
        glEnableVertexAttribArray(0);
        // color
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, vpsize, (void*)offsetof(VertexPrimitive, Color));
        glEnableVertexAttribArray(1);
        // normal
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, vpsize, (void*)offsetof(VertexPrimitive, Normal));
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_lines.size() * sizeof(unsigned int), m_lines.data(), GL_STATIC_DRAW);
    }

    void ArrowDrawable::_draw() {
        m_shader->setBool("ignoreLight", true);
        glLineWidth(2.0f);
        if (m_ebo != 0 && !m_lines.empty()) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
            glDrawElements(GL_LINES, m_lines.size(), GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(GL_LINES, 0, m_vertices.size());
        }
    }
}
