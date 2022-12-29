#ifndef RENDERER_H
#define RENDERER_H

#include <memory>
#include <GLFW/glfw3.h>

namespace RenderSpace {
    class RenderEngine;
    class RenderContext;

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
        GLFWwindow* m_window;
    
    private:
        RenderEngine& m_engine;

        std::shared_ptr<RenderContext> m_context;
    };
}

#endif
