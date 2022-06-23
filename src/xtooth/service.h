#ifndef XSERVICE_H
#define XSERVICE_H

#include <vector>
#include <string>
#include <memory>
#include "../infrastructure/communication/ContextHub.h"
#include "../infrastructure/communication/AutoBus.hpp"
#include "../render/mesh/typedef.hpp"

namespace XToothSpace {
    class XToothEngine;

    class XToothService {
    public:
        XToothService() = delete;
        explicit XToothService(
            XToothEngine& engine,
            const std::string& name
        ) noexcept;
        explicit XToothService(XToothEngine& engine) noexcept;

        ~XToothService();

        void simulate();

    private:
        void _init(const std::string& name);

        void _reset();

        void _subscribe();

        void _pick_from_ray(
            const RenderSpace::Point& ori,
            const RenderSpace::Direction& dir
        );
        void _clear_arrow();
        
        template <class ...Args>
        static inline std::string fmt_str(const char* fmt, Args... args) {
            char buffer[1024] = ""; // preset
            snprintf(buffer, sizeof(buffer), fmt, args...);
            return std::move(std::string(buffer));
        }

    private:
        std::string m_name; // 名称
        XToothEngine& m_engine;

        std::vector<int> m_arrow_ids;

        std::unique_ptr<fundamental::AutoBus> m_autobus;
    };
}

#endif
