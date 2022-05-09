#ifndef RENDERER_H
#define RENDERER_H

#include "shader.hpp"
#include "xwindow.h"
#include "../infrastructure/communication/ContextHub.h"
#include "../infrastructure/communication/AutoBus.hpp"

namespace RenderSpace {
    class RenderEngine;

    class Renderer {
    public:
        Renderer(RenderEngine& engine, unsigned int width, unsigned int height);
        ~Renderer();

        int exec();
    
    private:
        void setup();

    private:
        unsigned int m_vao;
        unsigned int m_vbo;
        Shader m_shader;
        RenderWindowWidget m_win_widget;
        GLFWwindow* m_window;

        std::unique_ptr<fundamental::AutoBus> m_autobus;
    
    private:
        RenderEngine& m_engine;
    };
}

#endif
