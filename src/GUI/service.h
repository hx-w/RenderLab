#ifndef GUI_SERVICE_H
#define GUI_SERVICE_H

#include <string>
#include <memory>
#include <communication/AutoBus.hpp>

namespace GUISpace {

    class GUIEngine;
    class GUIService {
    public:
        GUIService() = delete;
        explicit GUIService(GUIEngine& engine) noexcept;

        ~GUIService();
    
    public:
        /// [Slots]
        void slot_load_mesh(const std::string&);
       
        /// generic notify
        template <class Func, class ...Args>
        void notify(const std::string& addr, Args&& ...args) {
            auto _event = fundamental::ContextHub::getInstance()->getEventTable<Func>();
            _event->notify(m_symbol + addr, std::forward<Args>(args)...);
        }

    private:
        void _subscribe_all();

    private:
        std::string m_symbol = "GUI";
        GUIEngine& m_engine;
        std::unique_ptr<fundamental::AutoBus> m_autobus;
    };
}


#endif
