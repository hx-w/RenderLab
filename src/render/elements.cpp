#include "elements.h"

namespace RenderSpace {
    void RenderVertices::add_vertex(const Point& pnt, const Point& clr) {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_vertices.emplace_back(Vertex{
            glm::vec3(pnt.x(), pnt.y(), pnt.z()),
            glm::vec3(clr.x(), clr.y(), clr.z())
        });
        m_vertex_count ++;
    }
}