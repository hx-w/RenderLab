#ifndef RENDER_ELEMENTS_H
#define RENDER_ELEMENTS_H

#include "../libs/coords.h"

namespace RenderSpace {
    #define MAX_VERTEX_COUNT 1 << 16
    struct RenderVertices {
        RenderVertices() = default;
        ~RenderVertices() = default;

        void add_vertex(const Point& pnt, const Point& clr);

        unsigned int m_vertex_count = 0;
        float m_vertices[MAX_VERTEX_COUNT];
    };
}

#endif
