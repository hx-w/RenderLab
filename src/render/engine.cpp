#include "engine.h"

using namespace std;

namespace RenderSpace {
    RenderEngine* RenderEngine::m_instance = nullptr;
    std::once_flag RenderEngine::m_inited;

    RenderEngine* RenderEngine::get_instance() {
        std::call_once(m_inited, []() {
            m_instance = new RenderEngine();
        });
        return m_instance;
    }

    void RenderEngine::destroy() {
        if (m_instance) {
            delete m_instance;
            m_instance = nullptr;
        }
    }

    void RenderEngine::terminate() {
        for (auto render : m_renders) {
            delete render;
        }
        m_renders.clear();
    }

    Renderer* RenderEngine::create_render(unsigned int width, unsigned int height) {
        auto render = new Renderer(*this, width, height);
        m_renders.emplace(render);
        return render;
    }

    void RenderEngine::destroy_render(Renderer* render) {
        auto iter = m_renders.find(render);
        if (iter != m_renders.end()) {
            delete* iter;
            m_renders.erase(iter);
        }
    }

    Renderer* make_renderer(unsigned int width, unsigned int height) {
        return RenderEngine::get_instance()->create_render(width, height);
    }
}
