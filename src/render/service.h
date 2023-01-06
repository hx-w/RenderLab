#ifndef RENDER_SERVICE_H
#define RENDER_SERVICE_H

#include <memory>
#include <string>
#include <array>
#include <unordered_map>
#include <functional>
#include <stack>
#include <communication/AutoBus.hpp>
#include <communication/ContextHub.h>

#include "xwindow.h"
#include "shader.h"
#include "mesh/elements.h"
namespace RenderSpace {
    class RenderWindowWidget;
    class RenderContext;
#if defined(_WIN32)
    typedef std::unordered_map<std::string, std::thread::native_handle_type> ThreadMap;
#else
    typedef std::unordered_map<std::string, pthread_t> ThreadMap;
#endif

    class RenderService {
    public:
        RenderService();
        ~RenderService();

        void init_context(std::shared_ptr<RenderContext>);

        void register_methods();

        void ray_pick(const glm::vec3& origin, const glm::vec3& direction);
        void notify_clear_picking(); // refresh all picking ray
        void notify_window_resize(uint32_t width, uint32_t height);

        template <class Func, class ...Args>
        void notify(const std::string& addr, Args&& ...args) {
            auto _event = fundamental::ContextHub::getInstance()->getEventTable<Func>();
            _event->notify(m_symbol + addr, std::forward<Args>(args)...);
        }

    private:
        void start_thread(std::string tname, std::function<void()>&& func);

    private:
        // 网格列表
        std::unordered_map<int, std::shared_ptr<MeshDrawable>> m_meshes_map;

        // context
        std::shared_ptr<RenderContext> m_context;

        std::string m_symbol = "render";
        std::unique_ptr<fundamental::AutoBus> m_autobus;

        int m_id_gen = 0;
        std::mutex m_mutex;

        std::stack<int> m_wait_deleted;

        // thread manage
        ThreadMap m_thread_map;
    };
}

#endif
