#include "service.h"

#include <thread>
#include <communication/ContextHub.h>

#include "xwindow.h"
#include "context.h"

using namespace std;
using namespace geometry;
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
                return m_context->ctx_load_drawable(file_path);
            }
        );
        m_autobus->registerMethod<bool(uint32_t)>(
            m_symbol + "/remove_drawable",
            [this](uint32_t draw_id) -> bool {
                return m_context->ctx_remove_drawable(draw_id);
            }
        );
        /// @brief get drawble instance
        ///       THIS COULD BE DANGEROUS
        m_autobus->registerMethod<shared_ptr<DrawableBase>(DrawableID)>(
            m_symbol + "/get_drawable_inst",
            [this](DrawableID msh_id) {
                return m_context->ctx_get_drawable(msh_id);
            }
        );
        m_autobus->registerMethod<bool(DrawableID, const string&, const std::any&)>(
            m_symbol + "/set_drawable_property",
            [this](DrawableID msh_id, const string& property, const std::any& value) {
                return m_context->ctx_set_drawable_property(msh_id, property, value);
            }
        );
        m_autobus->registerMethod<void(int)>(
            m_symbol + "/set_interact_mode",
            [this](int mode) {
                m_context->ctx_change_interact_mode(mode);
            }
        );
        m_autobus->registerMethod<DrawableID(shared_ptr<GeometryBase>, Props&, int)>(
            m_symbol + "/add_geometry",
            [this](shared_ptr<GeometryBase> geom, Props& props, int type) -> DrawableID {
                return m_context->ctx_add_drawable(geom, props, type);
            }
		);
        m_autobus->registerMethod<void(const glm::mat4&)>(
            m_symbol + "/update_transform_mat",
            [this](const glm::mat4& transf) {
                m_context->ctx_update_transform_mat(transf);
            }
        );
    }

    void RenderService::start_thread(string tname, function<void()>&& func) {
        std::thread thrd = std::thread(func);
        m_thread_map[tname] = thrd.native_handle();
        thrd.detach();
        std::cout << "[INFO] thread " << tname << " created" << std::endl;
    }

    void RenderService::slot_add_log(const string& type, const string& msg) {
        auto _service = ContextHub::getInstance()->getServiceTable<void(const string&, const string&)>();
        _service->async_invoke("GUI/add_log", type, msg);
    }
}
