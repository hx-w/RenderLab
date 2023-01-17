#include "service.h"

#include <iostream>
#include <communication/ContextHub.h>

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
}
