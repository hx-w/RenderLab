#include "ContextHub.h"

namespace fundamental {
    ContextHub* ContextHub::m_pInstance = nullptr;
    std::once_flag ContextHub::m_inited;

    ContextHub* ContextHub::getInstance() {
        std::call_once(m_inited, []() {
            m_pInstance = new ContextHub;
        });
        return m_pInstance;
    }

    void ContextHub::destroy() {
        if (m_pInstance) {
            delete m_pInstance;
            m_pInstance = nullptr;
        }
    }
}