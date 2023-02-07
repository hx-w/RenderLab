#include "service.h"

#include <iostream>
#include <communication/ContextHub.h>
#include <line.hpp>

#include "workflow.h"
#include "wkflow_context.h"
#include "tooth_pack.h"
#include <geom_ext/drawable.h>
#include <mesh.h>

using namespace std;
using namespace fundamental;
using namespace RenderSpace;

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
        m_autobus->subscribe<void()>(SignalPolicy::Async, "render/render_setup",
            bind(&Workspace::init_workspace, m_workspace.get()));
        m_autobus->subscribe<void(const string&)>(SignalPolicy::Async, "GUI/filepath_selected",
            bind(&Workspace::fetch_filepath, m_workspace.get(), ::placeholders::_1, false));
        m_autobus->subscribe<void(const string&)>(SignalPolicy::Async, "render/filepath_dropin",
            bind(&Workspace::fetch_filepath, m_workspace.get(), ::placeholders::_1, false));
        m_autobus->subscribe<void(const string&, int)>(SignalPolicy::Async, "GUI/modal_confirm_feedback",
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
        m_autobus->subscribe<void(int, int /* status */)>(SignalPolicy::Async, "GUI/confirm_workflow",
            bind(&Workspace::confirm_workflow, m_workspace.get(), ::placeholders::_1, ::placeholders::_2));
    
        m_autobus->subscribe<void(int, int)>(SignalPolicy::Async, "GUI/active_workflow_stage",
            bind(&Workspace::active_stage, m_workspace.get(), ::placeholders::_1, ::placeholders::_2));

        m_autobus->subscribe<void(vector<uint32_t>&, vector<geometry::Point3f>&, vector<geometry::Vector3f>&)>(
            SignalPolicy::Async, "render/picked_points",
            bind(&Workspace::pick_points_handler, m_workspace.get(),
                ::placeholders::_1, ::placeholders::_2, ::placeholders::_3));
        m_autobus->subscribe<void(uint32_t, uint32_t)>(SignalPolicy::Async, "render/picked_vertex",
            bind(&Workspace::pick_vertex_handler, m_workspace.get(), ::placeholders::_1, ::placeholders::_2, true));
        m_autobus->subscribe<void(vector<vector<geometry::Point3f>>&, const std::pair<int, int>&)>(SignalPolicy::Sync, "GUI/send_nurbs_points_pack",
            bind(&Workspace::compute_nurbs_reverse, m_workspace.get(), ::placeholders::_1, ::placeholders::_2));

        m_autobus->subscribe<void(const vector<uint32_t>&)>(SignalPolicy::Async, "GUI/generate_depth",
            bind(&Workspace::generate_depth, m_workspace.get(), ::placeholders::_1));
        m_autobus->subscribe<void(const string&)>(SignalPolicy::Async, "GUI/set_heatmap_style",
            bind(&Workspace::set_heatmap_style, m_workspace.get(), ::placeholders::_1));
    }

    void ToothService::slot_add_log(const string& type, const string& msg) {
        auto _service = ContextHub::getInstance()->getServiceTable<void(const string&, const string&)>();
        _service->async_invoke("GUI/add_log", type, msg);
    }

    void ToothService::slot_add_notice(const string& name, const string& notice) {
        auto _service = ContextHub::getInstance()->getServiceTable<void(const string&, const string&)>();
        _service->async_invoke("GUI/add_notice", name, notice);
    }

    void ToothService::slot_open_workflow(WkflowCtxPtr ptr_params) {
        auto _service = ContextHub::getInstance()->getServiceTable<void(WkflowCtxPtr)>();
        _service->async_invoke("GUI/open_workflow", ptr_params);
    }

    void ToothService::slot_add_tooth_pack(shared_ptr<ToothPack> tpack_ptr) {
        auto _service = ContextHub::getInstance()->getServiceTable<void(shared_ptr<ToothPack>)>();
        _service->async_invoke("GUI/add_tooth_pack", tpack_ptr);
    }

    uint32_t ToothService::slot_load_mesh(const string& meshpath) {
        auto _service = ContextHub::getInstance()->getServiceTable<uint32_t(const string&)>();
        return _service->sync_invoke("render/load_mesh", meshpath);
    }

    bool ToothService::slot_set_drawable_property(uint32_t msh_id, const string& prop, const any& val) {
        auto _service = ContextHub::getInstance()->getServiceTable<bool(uint32_t, const string&, const any&)>();
        return _service->sync_invoke("render/set_drawable_property", msh_id, prop, val);
    }

    uint32_t ToothService::slot_show_arrow(geometry::Ray& ray, float len, geometry::Vector3f&& clr) {
        auto geom_ptr = make_shared<geometry::Ray>(ray);
        auto props = map<string, any>{
            {"color", clr }, {"arrow_length", len}
        };

        auto _service = ContextHub::getInstance()->getServiceTable<uint32_t(shared_ptr<geometry::GeometryBase>, map<string, any>&, int)>();
        return _service->sync_invoke("render/add_geometry", geom_ptr, props, 1);
    }

    uint32_t ToothService::slot_add_mesh(geometry::Mesh& mesh) {
        auto geom_ptr = make_shared<geometry::Mesh>(mesh);
        auto props = map<string, any>{
            {"color", geometry::Vector3f(0.5f)}
        };
        auto _service = ContextHub::getInstance()->getServiceTable<uint32_t(shared_ptr<geometry::GeometryBase>, map<string, any>&, int)>();
        return _service->sync_invoke("render/add_geometry", geom_ptr, props, 2);
    }

    void ToothService::slot_update_transform(const glm::mat4& transf) {
        auto _service = ContextHub::getInstance()->getServiceTable<void(const glm::mat4&)>();
        _service->async_invoke("render/update_transform_mat", transf);
    }

    shared_ptr<DrawableBase> ToothService::slot_get_drawable_inst(uint32_t draw_id) {
        auto _service = ContextHub::getInstance()->getServiceTable<shared_ptr<DrawableBase>(uint32_t)>();
        return _service->sync_invoke("render/get_drawable_inst", draw_id);
    }

    void ToothService::slot_set_mouse_tooltip(const string& tooltip) {
        auto _service = ContextHub::getInstance()->getServiceTable<void(const string&)>();
        _service->async_invoke("GUI/set_mouse_tooltip", tooltip);
    }

    uint32_t ToothService::slot_get_current_flow_id() {
        auto _service = ContextHub::getInstance()->getServiceTable<uint32_t()>();
        return _service->sync_invoke("GUI/get_current_flow_id");
    }
}
