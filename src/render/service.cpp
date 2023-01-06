#include "service.h"

#include <thread>

#include "xwindow.h"
#include "context.h"

using namespace std;
using namespace fundamental;

namespace RenderSpace {
    RenderService::RenderService():
        m_autobus(make_unique<AutoBus>()) {
        register_methods();
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

    void RenderService::register_methods() {
        /// @brief load_mesh
        m_autobus->registerMethod<uint32_t(const string&)>(
            m_symbol + "/load_mesh",
            [this](const string& file_path) -> uint32_t {
                return this->m_context->ctx_load_drawable(file_path);
            }
        );
    }

    void RenderService::ray_pick(const glm::vec3& ori, const glm::vec3& dir) {
        for (auto& [id, ptr]: m_meshes_map) {
            ptr->pick_cmd(ori, dir, 1e-1f);
        }
    }

    void RenderService::notify_clear_picking() {
        auto _event = ContextHub::getInstance()->getEventTable<void()>();
        _event->notify(m_symbol + "/clear_picking");
    }

    void RenderService::notify_window_resize(uint32_t width, uint32_t height) {
    }

    void RenderService::start_thread(string tname, function<void()>&& func) {
        std::thread thrd = std::thread(func);
        m_thread_map[tname] = thrd.native_handle();
        thrd.detach();
        std::cout << "[INFO] thread " << tname << " created" << std::endl;
    }
}
