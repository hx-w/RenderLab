#include "drawable.h"

#include "../libs/glad/glad.h"
#include "../libs/GLFW/glfw3.h"

using namespace std;

namespace RenderSpace {
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
        m_shader = shader;
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

    void Drawable::_deepcopy(const Drawable& element) {
        std::lock_guard<std::mutex> lk(m_mutex);
        _ready_to_draw = element._ready_to_draw;
        _ready_to_update = element._ready_to_update;
        m_triangles.assign(element.m_triangles.begin(), element.m_triangles.end());
        m_edges.assign(element.m_edges.begin(), element.m_edges.end());
        m_vertices.assign(element.m_vertices.begin(), element.m_vertices.end());
        m_center = element.m_center;
        m_radius = element.m_radius;
        m_aabb = element.m_aabb;
    }
}
