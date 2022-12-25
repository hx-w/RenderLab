#ifndef SERVICE_H
#define SERVICE_H

#include <vector>
#include <string>
#include <memory>
#include <communication/ContextHub.h>
#include <communication/AutoBus.hpp>

namespace ToothSpace {
    class ToothEngine;

    class ToothService {
    public:
        ToothService() = delete;
        explicit ToothService(
            ToothEngine& engine,
            const std::string& dir, int scale=100
        ) noexcept;
        explicit ToothService(ToothEngine& engine) noexcept;

        ~ToothService();
    private:
        void _subscribe();
        
        template <class ...Args>
        static inline std::string fmt_str(const char* fmt, Args... args) {
            char buffer[1024]; // preset
            snprintf(buffer, sizeof(buffer), fmt, args...);
            return std::move(std::string(buffer));
        }

    private:
        std::string m_name; // 名称
        ToothEngine& m_engine;

        std::unique_ptr<fundamental::AutoBus> m_autobus;
    };
}

#endif
