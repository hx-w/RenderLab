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
        // setup_pre -> init context -> setup_post
        void setup_pre(unsigned int w, unsigned int h);
        void setup_post();
        void update_transform();
        void draw();

    private:
        GLFWwindow* m_window;
    
        GLuint m_background_vao;

    private:
        RenderEngine& m_engine;

        std::shared_ptr<RenderContext> m_context;
    };
}

#endif
