#ifndef DRAWABLE_H
#define DRAWABLE_H
#include <mutex>
#include <vector>
#include "shader.hpp"
#include "../libs/coords.h"
#include "./libs/glm/glm.hpp"
#include "./libs/glm/gtc/matrix_transform.hpp"
#include "./libs/glm/gtc/type_ptr.hpp"

namespace RenderSpace {
    typedef glm::vec3 Normal;
    typedef std::pair<glm::vec3, glm::vec3> AABB; // min, max

    struct Vertex {
        Vertex() = default;
        Vertex(const glm::vec3& pos, const glm::vec3& clr=glm::vec3(0.0f, 0.0f, 0.0f)) :
            Position(pos), Color(clr) {
        }
        glm::vec3 Position;
        glm::vec3 Color;
    };

    struct Triangle {
        Triangle() = default;
        Triangle(const int v0, const int v1, const int v2) {
            VertexIdx = glm::uvec3(v0, v1, v2);
        }
        glm::uvec3 VertexIdx;
    };

    // base class for drawable objects
    class Drawable {
    public:
        Drawable();
        virtual ~Drawable();
        virtual void draw() = 0;

        void set_shader(Shader& shader);
    protected:
        void _gen_vao();
        void _reset();

    protected:
        Shader m_shader;
        unsigned int m_vao;
        unsigned int m_vbo;

        std::vector<Triangle> m_triangles;
        std::vector<Vertex> m_vertices;
        std::vector<Normal> m_normals;
        glm::vec3 m_center;
        float m_radius;
        AABB m_aabb;

        std::mutex m_mutex;
    };
}

#endif
