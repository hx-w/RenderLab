#ifndef RENDER_SERVICE_H
#define RENDER_SERVICE_H

#include <memory>
#include <string>
#include "shader.hpp"
#include "elements.h"
#include "../infrastructure/communication/AutoBus.hpp"

namespace RenderSpace {
    #define MAX_VERTEX_COUNT 1 << 16
    class RenderService {
    public:
        RenderService();
        ~RenderService() = default;

        // 临时画点集，后续删掉
        RenderVertices& get_vertices() { return m_vertices; }

        // 只画一个mesh
        Mesh& get_mesh() { return m_mesh; }

        MeshDrawable& get_meshdraw() { return m_meshdraw; }

        Shader& get_shader() { return m_shader; }

    private:
        void setup();

    private:
        RenderVertices m_vertices;
        Mesh m_mesh;

        MeshDrawable m_meshdraw;

        Shader m_shader;

        std::string m_symbol = "render";
        std::unique_ptr<fundamental::AutoBus> m_autobus;
    };
}

#endif
