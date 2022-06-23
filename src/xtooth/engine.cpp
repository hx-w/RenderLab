#include "engine.h"

using namespace std;

namespace XToothSpace {
    XToothEngine* XToothEngine::m_instance = nullptr;
    std::once_flag XToothEngine::m_inited;

    XToothEngine* XToothEngine::get_instance() {
        std::call_once(m_inited, []() {
            m_instance = new XToothEngine();
        });
        return m_instance;
    }

    void XToothEngine::destroy() {
        if (m_instance) {
            delete m_instance;
            m_instance = nullptr;
        }
    }

    void XToothEngine::terminate() {
        for (auto service : m_services) {
            delete service;
        }
        m_services.clear();
    }

    XToothService* XToothEngine::create_service(const string& name) {
        auto service = new XToothService(*this, name);
        m_services.emplace(service);
        return service;
    }

    void XToothEngine::destroy_service(XToothService* service) {
        auto iter = m_services.find(service);
        if (iter != m_services.end()) {
            delete* iter;
            m_services.erase(iter);
        }
    }

    XToothService* make_service(const string& name) {
        return XToothEngine::get_instance()->create_service(name);
    }
}
