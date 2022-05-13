#ifndef DRAWABLE_H
#define DRAWABLE_H
#include <mutex>
#include <vector>
#include <string>
#include "../shader.hpp"
#include "../libs/glm/glm.hpp"
#include "../libs/glm/gtc/matrix_transform.hpp"
#include "../libs/glm/gtc/type_ptr.hpp"

namespace RenderSpace {
    typedef std::pair<glm::vec3, glm::vec3> AABB; // min, max

    struct Vertex {
        Vertex() = default;
        Vertex(
            const glm::vec3& pos,
            const glm::vec3& clr,
            const glm::vec3& nml) :
            Position(pos), Color(clr), Normal(nml) { }
        glm::vec3 Position;
        glm::vec3 Color;
        glm::vec3 Normal;
    };

    // 独立边
    struct Edge {
        Edge() = default;
        Edge(const glm::vec3& v0, const glm::vec3& v1, int v0idx, int v1idx):
            v0(v0), v1(v1), v0idx(v0idx), v1idx(v1idx) { }
        Edge(const Edge& edge): v0(edge.v0), v1(edge.v1), v0idx(edge.v0idx), v1idx(edge.v1idx) { }
        bool operator==(const Edge& rhs) const {
            return (v0 == rhs.v0 && v1 == rhs.v1) || (v0 == rhs.v1 && v1 == rhs.v0);
        }
        bool operator<(const Edge& rhs) const {
            return v0.x < rhs.v0.x;
        }

        glm::vec3 v0;
        glm::vec3 v1;
        int v0idx;
        int v1idx;
    };

    struct Triangle {
        Triangle() = default;
        Triangle(const int v0, const int v1, const int v2):
            VertexIdx(glm::uvec3(v0, v1, v2)) { }
        glm::uvec3 VertexIdx;
    };

    // base class for drawable objects
    class Drawable {
    public:
        Drawable() = default;
        virtual ~Drawable();
        virtual void draw() = 0;

        void set_shader(Shader& shader);
        std::string get_name() const {
            return m_name;
        };

        std::vector<Triangle>& get_triangles() {
            return m_triangles;
        }
        std::vector<Vertex>& get_vertices() {
            return m_vertices;
        }
    protected:
        void _gen_vao();
        void _reset();

    protected:
        Shader m_shader;
        GLuint m_vao = 0;
        GLuint m_vbo = 0;
        GLuint m_ebo = 0;

        std::vector<Triangle> m_triangles;
        std::vector<Vertex> m_vertices;
        glm::vec3 m_center;
        float m_radius;
        AABB m_aabb;

        std::string m_name;
        std::mutex m_mutex;
    };
}

#endif
