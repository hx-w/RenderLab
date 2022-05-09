#ifndef RENDER_SERVICE_H
#define RENDER_SERVICE_H

#include <memory>
#include <string>
#include "elements.h"
#include "../infrastructure/communication/AutoBus.hpp"

namespace RenderSpace {
    #define MAX_VERTEX_COUNT 1 << 16
    class RenderService {
    public:
        RenderService();
        ~RenderService() = default;

        RenderVertices& get_vertices();

    private:
        void setup();

    private:
        RenderVertices m_vertices;

        std::string m_symbol = "render";
        std::unique_ptr<fundamental::AutoBus> m_autobus;
    };
}

#endif
