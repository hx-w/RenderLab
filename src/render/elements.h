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
