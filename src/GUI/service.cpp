#include "service.h"
#include "engine.h"
#include "viewer.h"

#include <functional>
#include <communication/ContextHub.h>

using namespace std;
using namespace fundamental;

namespace GUISpace {
    GUIService::GUIService(GUIEngine& engine) noexcept:
        m_engine(engine), m_autobus(make_unique<AutoBus>()) {
        _subscribe_all();
    }

    GUIService::~GUIService() {
        m_autobus.reset();
    }

    void GUIService::_subscribe_all() {
        m_autobus->subscribe<void()>(SignalPolicy::Sync, "render/pre-redraw",
            bind(&GUISpace::test));
    }
}