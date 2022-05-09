#include "service.h"
#include "../libs/coords.h"

using namespace std;
using namespace fundamental;

namespace RenderSpace {
    RenderService::RenderService():
        m_autobus(make_unique<AutoBus>()) {
        setup();
    }

    void RenderService::setup() {
        // 加点
        m_autobus->registerMethod<void(const Point& pnt, const Point& clr)>(
            m_symbol + "/add_point",
            [this](const Point& pnt, const Point& clr) {
                m_vertices.add_vertex(pnt, clr);
            });
    }

    RenderVertices& RenderService::get_vertices() {
        return m_vertices;
    }
}