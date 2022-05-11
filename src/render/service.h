#ifndef RENDER_SERVICE_H
#define RENDER_SERVICE_H

#include <memory>
#include <string>
#include "shader.hpp"
#include "elements.h"
#include "../infrastructure/communication/AutoBus.hpp"

namespace RenderSpace {
    class RenderService {
    public:
        RenderService();
        ~RenderService() = default;

        // 临时画点集，后续删掉
        RenderVertices& get_vertices() { return m_vertices; }

        MeshDrawable& get_meshdraw() { return m_meshdraw; }

        Shader& get_shader() { return m_shader; }

    private:
        void setup();

    private:
        RenderVertices m_vertices;
        MeshDrawable m_meshdraw;

        Shader m_shader;

        std::string m_symbol = "render";
        std::unique_ptr<fundamental::AutoBus> m_autobus;
    };
}

#endif
