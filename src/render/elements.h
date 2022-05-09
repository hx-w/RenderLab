#ifndef RENDER_ELEMENTS_H
#define RENDER_ELEMENTS_H

#include <mutex>
#include <vector>
#include "../libs/coords.h"
#include "./libs/glm/glm.hpp"
#include "./libs/glm/gtc/matrix_transform.hpp"
#include "./libs/glm/gtc/type_ptr.hpp"

namespace RenderSpace {
    struct Vertex {
        glm::vec3 Position;
        glm::vec3 Color;
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
}

#endif
