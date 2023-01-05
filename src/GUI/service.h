#ifndef GUI_SERVICE_H
#define GUI_SERVICE_H

#include <memory>
#include <communication/AutoBus.hpp>

namespace GUISpace {

    class GUIEngine;
    class GUIService {
    public:
        GUIService() = delete;
        explicit GUIService(GUIEngine& engine) noexcept;

        ~GUIService();
    
    private:
        void _subscribe_all();

    private:
        std::string m_symbol = "GUI";
        GUIEngine& m_engine;
        std::unique_ptr<fundamental::AutoBus> m_autobus;
    };
}


#endif
