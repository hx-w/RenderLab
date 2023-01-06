#include "engine.h"
#include "service.h"

using namespace std;

namespace GUISpace {
    GUIEngine* GUIEngine::m_instance = nullptr;
    std::once_flag GUIEngine::m_inited;

    GUIEngine* GUIEngine::get_instance() {
        std::call_once(m_inited, []() {
            m_instance = new GUIEngine();
        });
        return m_instance;
    }

    void GUIEngine::destroy() {
        if (m_instance) {
            delete m_instance;
            m_instance = nullptr;
        }
    }

    void GUIEngine::terminate() {
        for (auto service : m_services) {
            delete service;
        }
        m_services.clear();
    }

    GUIService* GUIEngine::create_service() {
        auto service = new GUIService(*this);
        m_services.emplace(service);
        return service;
    }

    void GUIEngine::destroy_service(GUIService* service) {
        auto iter = m_services.find(service);
        if (iter != m_services.end()) {
            delete* iter;
            m_services.erase(iter);
        }
    }

    GUIService* GUIEngine::get_service(int index) {
        if (m_services.size() <= index) return nullptr;
        return *m_services.begin();
    }

    GUIService* make_service() {
        return GUIEngine::get_instance()->create_service();
    }
}