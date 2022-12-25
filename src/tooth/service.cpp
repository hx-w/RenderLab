#include "service.h"
#include "printer.h"
#include "execute.h"
#include <cmath>
#include <functional>
#include <array>
#include <string>
#include <regex>

using namespace std;
using namespace fundamental;

namespace ToothSpace {
    ToothService::ToothService(
        ToothEngine& engine,
        const string& dir, int scale
    ) noexcept : m_engine(engine), m_autobus(make_unique<AutoBus>()) {
        _subscribe();
    }

    ToothService::ToothService(ToothEngine& engine) noexcept
        : m_engine(engine) {}

    ToothService::~ToothService() {
        m_autobus.reset();
    }

    void ToothService::_subscribe() {
        // 拾取射线
        // m_autobus->subscribe<void(const Point&, const Direction&)>(SignalPolicy::Sync, "render/picking_ray",
        //     bind(&ToothService::_pick_from_ray, this, placeholders::_1, placeholders::_2));
        // 清除拾取射线
        // m_autobus->subscribe<void()>(SignalPolicy::Sync, "render/clear_picking",
        //     bind(&ToothService::_clear_arrow, this));
    }
}
