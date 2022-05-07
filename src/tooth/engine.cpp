#include "engine.h"

using namespace std;

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

    void ToothEngine::terminate() {
        for (auto service : m_services) {
            delete service;
        }
        m_services.clear();
    }

    ToothService* ToothEngine::create_service(const string& dir, int scale) {
        auto service = new ToothService(*this, dir, scale);
        m_services.emplace(service);
        return service;
    }

    void ToothEngine::destroy_service(ToothService* service) {
        auto iter = m_services.find(service);
        if (iter != m_services.end()) {
            delete* iter;
            m_services.erase(iter);
        }
    }
}