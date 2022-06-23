#ifndef XTOOTH_ENGINE_H
#define XTOOTH_ENGINE_H

#include <mutex>
#include <set>
#include "service.h"

namespace XToothSpace {
    // singleton
    // 包括所有service的资源管理
    class XToothEngine {
    public:
        static XToothEngine* get_instance();
        static void destroy();
        XToothEngine(const XToothEngine&) = delete;
        XToothEngine& operator=(const XToothEngine&) = delete;
        ~XToothEngine() { terminate(); };

        XToothService* create_service(const std::string& name);
        void destroy_service(XToothService*);

    private:
        XToothEngine() = default;
        void terminate();

    private:
        std::set<XToothService*> m_services;

    private:
        static XToothEngine* m_instance;
        static std::once_flag m_inited;
    };

    XToothService* make_service(const std::string& name);
}

#endif
