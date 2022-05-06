#ifndef TOOTH_ENGINE_H
#define TOOTH_ENGINE_H

#include <mutex>
#include "../libs/nurbs.h"

namespace ToothSpace {
    // singleton
    class ToothEngine {
    public:
        static ToothEngine* get_instance();
        static void destroy();
        ToothEngine(const ToothEngine&) = delete;
        ToothEngine& operator=(const ToothEngine&) = delete;
        ~ToothEngine() = default;

    private:
        ToothEngine() = default;

    private:
        static ToothEngine* m_instance;
        static std::once_flag m_inited;
    };
}

#endif
