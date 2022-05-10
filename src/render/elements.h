#ifndef RENDER_ELEMENTS_H
#define RENDER_ELEMENTS_H

#include "drawable.h"

namespace RenderSpace {
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

        // bool load_STL(const std::string& filename);

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
        // bool _read_STL_ASCII(const std::string& filename);
        // bool _read_STL_Binary(const std::string& filename);
    private:
        std::vector<Triangle> m_triangles;
        std::vector<Vertex> m_vertices;
        std::vector<Normal> m_normals;
        glm::vec3 m_center;
        float m_radius;

        std::mutex m_mutex;
    };

    class MeshDrawable : public Drawable {
    public:
        MeshDrawable() = default;
        ~MeshDrawable() = default;

        void draw() override;

        bool load_STL(const std::string& filename);
    private:
        bool _read_STL_ASCII(const std::string& filename);
        bool _read_STL_Binary(const std::string& filename);
    };
}

#endif
