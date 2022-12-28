#include "drawable.h"

#include <mesh.h>

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

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
    }

    void DrawableBase::draw() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!_ready_to_draw) {
            return;
        }
        m_shader.use();
        glBindVertexArray(m_vao);

        // shader configure
        ///1. local transform = identity
        m_shader.setMat4("model", glm::mat4(1.0f));
        ///2. model offset
        m_shader.setVec3("offset", m_offset);
        ///3. shade mode
        glPolygonMode(GL_FRONT_AND_BACK, m_shade_mode);
        ///4. randomColor
        m_shader.setBool("randomColor", false);

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
    }

    void NewMeshDrawable::_draw() {
        m_shader.setBool("ignoreLight", false);
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
        glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * vpsize, &m_vertices[0], GL_DYNAMIC_DRAW);
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
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_faces.size() * sizeof(geometry::Vector3u), &m_faces[0], GL_DYNAMIC_DRAW);
    }

}