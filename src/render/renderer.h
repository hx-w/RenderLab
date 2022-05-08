#ifndef RENDERER_H
#define RENDERER_H

#include "xwindow.h"

namespace RenderSpace {
    class Renderer {
    public:
        Renderer(unsigned int width = 800, unsigned int height = 600);
        ~Renderer();

        int exec();
    
    private:
        RenderWindowWidget m_win_widget;
        GLFWwindow* m_window;
    };
}

#endif
