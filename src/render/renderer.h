#ifndef RENDERER_H
#define RENDERER_H

#include "service.h"
#include "xwindow.h"
#include "container.h"

namespace RenderSpace {
    class RenderEngine;

    class Renderer {
    public:
        Renderer(RenderEngine& engine, unsigned int width, unsigned int height);
        ~Renderer();

        int exec();
    
        std::shared_ptr<RenderService> get_service();

    private:
        void setup(unsigned int w, unsigned int h);
        void update_transform();
        void draw();

    private:
        std::shared_ptr<RenderWindowWidget> m_win_widget;
        GLFWwindow* m_window;
    
    private:
        RenderEngine& m_engine;
        std::shared_ptr<RenderService> m_service;

        std::shared_ptr<RenderContainer> m_container;
    };
}

#endif
