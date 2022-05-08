#ifndef TOOTH_ENGINE_H
#define TOOTH_ENGINE_H

#include <mutex>
#include <set>
#include "service.h"

namespace ToothSpace {
    // singleton
    // 包括所有service的资源管理
    class ToothEngine {
    public:
        static ToothEngine* get_instance();
        static void destroy();
        ToothEngine(const ToothEngine&) = delete;
        ToothEngine& operator=(const ToothEngine&) = delete;
        ~ToothEngine() { terminate(); };

        ToothService* create_service(const std::string& dir, int scale=100);
        void destroy_service(ToothService*);

    private:
        ToothEngine() = default;
        void terminate();

    private:
        std::set<ToothService*> m_services;

    private:
        static ToothEngine* m_instance;
        static std::once_flag m_inited;
    };
}

#endif
