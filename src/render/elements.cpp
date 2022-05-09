#include "elements.h"

namespace RenderSpace {
    void RenderVertices::add_vertex(const Point& pnt, const Point& clr) {
        if (m_vertex_count * 3 >= MAX_VERTEX_COUNT) {
            return;
        }
        m_vertices[m_vertex_count * 3] = pnt.x();
        m_vertices[m_vertex_count * 3 + 1] = pnt.y();
        m_vertices[m_vertex_count * 3 + 2] = pnt.z();
        m_vertex_count ++;
    }
}