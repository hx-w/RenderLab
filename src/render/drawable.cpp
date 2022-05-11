#include "drawable.h"

#include "./libs/glad/glad.h"
#include "./libs/GLFW/glfw3.h"

using namespace std;

namespace RenderSpace {
    Drawable::Drawable() {
        // _gen_vao();
    }

    Drawable::~Drawable() {
        _reset();
    }

    void Drawable::set_shader(Shader& shader) {
        m_shader = shader;
    }

    void Drawable::_reset() {
        vector<Triangle>().swap(m_triangles);
        vector<Vertex>().swap(m_vertices);
        vector<Normal>().swap(m_normals);
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_vbo);
    }

    void Drawable::_gen_vao() {
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
    }
}
