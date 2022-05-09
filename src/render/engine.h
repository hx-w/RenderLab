#ifndef RENDER_ENGINE_H
#define RENDER_ENGINE_H

#include <mutex>
#include <set>
#include "renderer.h"

namespace RenderSpace {
    // singleton
    // 包括所有service的资源管理
    class RenderEngine {
    public:
        static RenderEngine* get_instance();
        static void destroy();
        RenderEngine(const RenderEngine&) = delete;
        RenderEngine& operator=(const RenderEngine&) = delete;
        ~RenderEngine() { terminate(); };

        Renderer* create_render(unsigned int width, unsigned int height);
        void destroy_render(Renderer*);

    private:
        RenderEngine() = default;
        void terminate();

    private:
        std::set<Renderer*> m_renders;

    private:
        static RenderEngine* m_instance;
        static std::once_flag m_inited;
    };

    Renderer* make_renderer(unsigned int width, unsigned int height);
}

#endif
