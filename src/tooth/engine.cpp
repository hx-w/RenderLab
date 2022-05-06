#include "engine.h"

namespace ToothSpace {
    ToothEngine* ToothEngine::m_instance = nullptr;
    std::once_flag ToothEngine::m_inited;

    ToothEngine* ToothEngine::get_instance() {
        std::call_once(m_inited, []() {
            m_instance = new ToothEngine();
        });
        return m_instance;
    }

    void ToothEngine::destroy() {
        if (m_instance) {
            delete m_instance;
            m_instance = nullptr;
        }
    }
}