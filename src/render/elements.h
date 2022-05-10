#ifndef RENDER_ELEMENTS_H
#define RENDER_ELEMENTS_H

#include <mutex>
#include <vector>
#include "../libs/coords.h"
#include "./libs/glm/glm.hpp"
#include "./libs/glm/gtc/matrix_transform.hpp"
#include "./libs/glm/gtc/type_ptr.hpp"

namespace RenderSpace {
    typedef glm::vec3 Normal;

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

    struct RenderVertices {
        RenderVertices() = default;
        ~RenderVertices() = default;

        void add_vertex(const Point& pnt, const Point& clr);

        unsigned int m_vertex_count = 0;
        std::vector<Vertex> m_vertices;
    private:
        std::mutex m_mutex;
    };

    class Mesh {
    public:
        Mesh() = default;
        ~Mesh();

        bool load_STL(const std::string& filename);

        std::vector<Triangle>& get_triangles() {
            return m_triangles;
        };
        std::vector<Vertex>& get_vertices() {
            return m_vertices;
        }
        std::vector<Normal>& get_normals() {
            return m_normals;
        }
    private:
        void _reset();
        bool _read_STL_ASCII(const std::string& filename);
        bool _read_STL_Binary(const std::string& filename);
    private:
        std::vector<Triangle> m_triangles;
        std::vector<Vertex> m_vertices;
        std::vector<Normal> m_normals;
        glm::vec3 m_center;
        float m_radius;

        std::mutex m_mutex;
    };
}

#endif
