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

        template <class Func, class ...Args>
        void notify(const std::string& addr, Args&& ...args) {
            auto _event = fundamental::ContextHub::getInstance()->getEventTable<Func>();
            _event->notify(m_symbol + addr, std::forward<Args>(args)...);
        }

    public:
        /// [Slots]
        void slot_add_log(const std::string&, const std::string&);

        void slot_set_mouse_tooltip(const std::string&);

    private:
        void start_thread(std::string tname, std::function<void()>&& func);
        void _register_all();

    private:

        // context
        std::shared_ptr<RenderContext> m_context;

        std::string m_symbol = "render";
        std::unique_ptr<fundamental::AutoBus> m_autobus;

        std::mutex m_mutex;

        // thread manage
        ThreadMap m_thread_map;
    };
}

#endif
