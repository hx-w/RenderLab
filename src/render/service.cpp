#include "service.h"

#include <thread>
#include <communication/ContextHub.h>

#include "xwindow.h"
#include "context.h"

using namespace std;
using namespace fundamental;

namespace RenderSpace {
    RenderService::RenderService():
        m_autobus(make_unique<AutoBus>()) {
        _register_all();
    }

    RenderService::~RenderService() {
        for (auto& [tname, thr] : m_thread_map) {
#if !defined(_WIN32)
            pthread_cancel(thr);
#endif
            m_thread_map.erase(tname);
        }
    }

    void RenderService::init_context(std::shared_ptr<RenderContext> ctx) {
        m_context = ctx;
    }

    void RenderService::_register_all() {
        /// @brief load_mesh
        m_autobus->registerMethod<uint32_t(const string&)>(
            m_symbol + "/load_mesh",
            [this](const string& file_path) -> uint32_t {
                return this->m_context->ctx_load_drawable(file_path);
            }
        );
    }

    void RenderService::start_thread(string tname, function<void()>&& func) {
        std::thread thrd = std::thread(func);
        m_thread_map[tname] = thrd.native_handle();
        thrd.detach();
        std::cout << "[INFO] thread " << tname << " created" << std::endl;
    }

    void RenderService::slot_add_log(string&& type, const string& msg) {
        auto _service = ContextHub::getInstance()->getServiceTable<void(string&&, const string&)>();
        _service->sync_invoke("GUI/add_log", forward<string&&>(type), msg);
    }
}
