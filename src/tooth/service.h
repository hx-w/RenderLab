#ifndef TOOTH_SERVICE_H
#define TOOTH_SERVICE_H

#include <any>
#include <string>
#include <memory>
#include <communication/AutoBus.hpp>
#include <geom_types.h>

namespace ToothSpace {
    class ToothEngine;
    class Workspace;
    class ToothPack;
    struct WorkflowContext;

    class ToothService {
    public:
        ToothService() = delete;
        explicit ToothService(ToothEngine& engine) noexcept;

        ~ToothService();

    public:
        void slot_add_log(std::string&&, const std::string&);
        void slot_add_notice(const std::string&, const std::string&);
        void slot_open_workflow(std::shared_ptr<WorkflowContext>);
        void slot_add_tooth_pack(std::shared_ptr<ToothPack>);
        uint32_t slot_load_mesh(const std::string&);
        bool slot_set_drawable_property(uint32_t, const std::string&, const std::any&);

        // to renderer
        uint32_t slot_show_arrow(geometry::Ray&, float /* length */, geometry::Vector3f& /* color */);

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
