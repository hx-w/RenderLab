#ifndef TOOTH_SERVICE_H
#define TOOTH_SERVICE_H

#include <string>
#include <memory>
#include <communication/AutoBus.hpp>

namespace ToothSpace {
    class ToothEngine;
    class Workspace;

    class ToothService {
    public:
        ToothService() = delete;
        explicit ToothService(ToothEngine& engine) noexcept;

        ~ToothService();

    public:
        void slot_add_log(std::string&&, const std::string&);
        void slot_add_notice(const std::string&, const std::string&);

    private:
        void _subscribe();

    private:
        std::string m_name = "tooth"; // 名称
        ToothEngine& m_engine;

        std::unique_ptr<Workspace> m_workspace;
        std::unique_ptr<fundamental::AutoBus> m_autobus;
    };
}

#endif
