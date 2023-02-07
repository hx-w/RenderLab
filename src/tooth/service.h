#ifndef TOOTH_SERVICE_H
#define TOOTH_SERVICE_H

#include <any>
#include <string>
#include <memory>
#include <communication/AutoBus.hpp>
#include <geom_types.h>

namespace RenderSpace {
    class DrawableBase;
}

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
        void slot_add_log(const std::string&, const std::string&);
        void slot_add_notice(const std::string&, const std::string&);
        void slot_open_workflow(std::shared_ptr<WorkflowContext>);
        void slot_add_tooth_pack(std::shared_ptr<ToothPack>);
        uint32_t slot_load_mesh(const std::string&);
        bool slot_set_drawable_property(uint32_t, const std::string&, const std::any&);

        // to renderer
        uint32_t slot_show_arrow(geometry::Ray&, float /* length */, geometry::Vector3f&& /* color */);

        uint32_t slot_add_mesh(geometry::Mesh&);

        uint32_t slot_get_current_flow_id();

        // to renderer
        void slot_update_transform(const glm::mat4&);

        std::shared_ptr<RenderSpace::DrawableBase> slot_get_drawable_inst(uint32_t);

        void slot_set_mouse_tooltip(const std::string&);

        template <class Func, class ...Args>
        void notify(const std::string& addr, Args&& ...args) {
            auto _event = fundamental::ContextHub::getInstance()->getEventTable<Func>();
            _event->notify(m_symbol + addr, std::forward<Args>(args)...);
        }

    private:
        void _subscribe();

    private:
        std::string m_symbol = "tooth"; // 名称
        ToothEngine& m_engine;

        std::unique_ptr<Workspace> m_workspace;
        std::unique_ptr<fundamental::AutoBus> m_autobus;
    };
}

#endif
