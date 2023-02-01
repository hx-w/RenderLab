#ifndef GUI_SERVICE_H
#define GUI_SERVICE_H

#include <string>
#include <memory>
#include <communication/AutoBus.hpp>

namespace RenderSpace {
    class DrawableBase;
}

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
       
        /// [Slots]
        std::shared_ptr<RenderSpace::DrawableBase> slot_get_drawable_inst(uint32_t);

        /// [Slots]
        bool slot_remove_drawable(uint32_t);

        /// generic notify
        template <class Func, class ...Args>
        void notify(const std::string& addr, Args&& ...args) {
            auto _event = fundamental::ContextHub::getInstance()->getEventTable<Func>();
            _event->notify(m_symbol + addr, std::forward<Args>(args)...);
        }

    private:
        void _subscribe_all();
        void _register_all();

    private:
        std::string m_symbol = "GUI";
        GUIEngine& m_engine;
        std::unique_ptr<fundamental::AutoBus> m_autobus;
    };
}


#endif
