#ifndef RENDERER_H
#define RENDERER_H

#include "service.h"
#include "xwindow.h"

namespace RenderSpace {
    class RenderEngine;

    class Renderer {
    public:
        Renderer(RenderEngine& engine, unsigned int width, unsigned int height);
        ~Renderer();

        int exec();
    
    private:
        void setup(unsigned int w, unsigned int h);
        void update_transform();
        void draw();

    private:
        RenderWindowWidget m_win_widget;
        GLFWwindow* m_window;
    
    private:
        RenderEngine& m_engine;
        std::shared_ptr<RenderService> m_service;
    };
}

#endif
