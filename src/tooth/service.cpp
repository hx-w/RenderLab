#include "service.h"

#include <iostream>
#include <communication/ContextHub.h>
#include <line.hpp>

#include "workflow.h"
#include "wkflow_context.h"
#include "tooth_pack.h"

using namespace std;
using namespace fundamental;

namespace ToothSpace {
    ToothService::ToothService(ToothEngine& engine) noexcept
        : m_engine(engine),
          m_autobus(make_unique<AutoBus>()),
          m_workspace(make_unique<Workspace>()) {
        _subscribe();
    }

    ToothService::~ToothService() {
        m_autobus.reset();
        m_workspace.reset();
    }

    void ToothService::_subscribe() {
        m_autobus->subscribe<void(const string&)>(SignalPolicy::Sync, "GUI/filepath_selected",
            bind(&Workspace::fetch_filepath, m_workspace.get(), ::placeholders::_1, false));
        m_autobus->subscribe<void(const string&)>(SignalPolicy::Sync, "render/filepath_dropin",
            bind(&Workspace::fetch_filepath, m_workspace.get(), ::placeholders::_1, false));
        m_autobus->subscribe<void(const string&, int)>(SignalPolicy::Sync, "GUI/modal_confirm_feedback",
            [this](const string& name, int res) {
                const string prefix = "Force to load the project?##";
                if (name._Starts_with(prefix)) { // filter just asked
                    // substr the backword
                    auto filepath = name.substr(prefix.length());
                    if (res) {
                        m_workspace->fetch_filepath(filepath, true);
                    }
                    else {
                        slot_add_log("error", "project load failed");
                    }
                }
            });
        m_autobus->subscribe<void(int, int /* status */)>(SignalPolicy::Sync, "GUI/confirm_workflow",
            bind(&Workspace::confirm_workflow, m_workspace.get(), ::placeholders::_1, ::placeholders::_2));
    
        m_autobus->subscribe<void(int, int)>(SignalPolicy::Sync, "GUI/active_workflow_stage",
            bind(&Workspace::active_stage, m_workspace.get(), ::placeholders::_1, ::placeholders::_2));

        m_autobus->subscribe<void(vector<uint32_t>&, vector<geometry::Point3f>&, vector<geometry::Vector3f>&)>(
            SignalPolicy::Sync, "render/picked_points",
            bind(&Workspace::pick_service_handler, m_workspace.get(),
                ::placeholders::_1, ::placeholders::_2, ::placeholders::_3));
    }

    void ToothService::slot_add_log(string&& type, const string& msg) {
        auto _service = ContextHub::getInstance()->getServiceTable<void(string&&, const string&)>();
        _service->sync_invoke("GUI/add_log", forward<string&&>(type), msg);
    }

    void ToothService::slot_add_notice(const string& name, const string& notice) {
        auto _service = ContextHub::getInstance()->getServiceTable<void(const string&, const string&)>();
        _service->sync_invoke("GUI/add_notice", name, notice);
    }

    void ToothService::slot_open_workflow(WkflowCtxPtr ptr_params) {
        auto _service = ContextHub::getInstance()->getServiceTable<void(WkflowCtxPtr)>();
        _service->sync_invoke("GUI/open_workflow", ptr_params);
    }

    void ToothService::slot_add_tooth_pack(shared_ptr<ToothPack> tpack_ptr) {
        auto _service = ContextHub::getInstance()->getServiceTable<void(shared_ptr<ToothPack>)>();
        _service->sync_invoke("GUI/add_tooth_pack", tpack_ptr);
    }

    uint32_t ToothService::slot_load_mesh(const string& meshpath) {
        auto _service = ContextHub::getInstance()->getServiceTable<uint32_t(const string&)>();
        return _service->sync_invoke("render/load_mesh", meshpath);
    }

    bool ToothService::slot_set_drawable_property(uint32_t msh_id, const string& prop, const any& val) {
        auto _service = ContextHub::getInstance()->getServiceTable<bool(uint32_t, const string&, const any&)>();
        return _service->sync_invoke("render/set_drawable_property", msh_id, prop, val);
    }

    uint32_t ToothService::slot_show_arrow(geometry::Ray& ray, float len, geometry::Vector3f& clr) {
        auto geom_ptr = make_shared<geometry::Ray>(ray);
        auto props = map<string, any>{
            {"color", clr }, {"arrow_length", len}
        };

        auto _service = ContextHub::getInstance()->getServiceTable<uint32_t(shared_ptr<geometry::GeometryBase>, map<string, any>&, int)>();
        return _service->sync_invoke("render/add_geometry", geom_ptr, props, 1);
    }
}
