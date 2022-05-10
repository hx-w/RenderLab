#ifndef RENDER_SERVICE_H
#define RENDER_SERVICE_H

#include <memory>
#include <string>
#include "elements.h"
#include "../infrastructure/communication/AutoBus.hpp"

namespace RenderSpace {
    #define MAX_VERTEX_COUNT 1 << 16
    class RenderService {
    public:
        RenderService();
        ~RenderService() = default;

        // 临时画点集，后续删掉
        RenderVertices& get_vertices();

        // 只画一个mesh
        Mesh& get_mesh();

    private:
        void setup();

    private:
        RenderVertices m_vertices;
        Mesh m_mesh;

        std::string m_symbol = "render";
        std::unique_ptr<fundamental::AutoBus> m_autobus;
    };
}

#endif
