#ifndef RENDER_SERVICE_H
#define RENDER_SERVICE_H

#include <memory>
#include <string>
#include "shader.hpp"
#include "./mesh/elements.h"
#include "../infrastructure/communication/AutoBus.hpp"

namespace RenderSpace {
    class RenderService {
    public:
        RenderService();
        ~RenderService() = default;

        Shader& get_shader() { return m_shader; }

        void draw_all();

        void update();

    private:
        void setup();

        void sync_all();

    private:
        MeshDrawable m_nurbs;
        MeshDrawable m_meshdraw;

        Shader m_shader;

        std::string m_symbol = "render";
        std::unique_ptr<fundamental::AutoBus> m_autobus;
    };
}

#endif
