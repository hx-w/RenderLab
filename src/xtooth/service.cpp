#include "service.h"
#include <cmath>
#include <functional>
#include <array>
#include <string>
#include <regex>

#include "../render/mesh/elements.h"

using namespace std;
using namespace fundamental;
using RenderSpace::Point;
using RenderSpace::Direction;
using RenderSpace::Drawable;

namespace XToothSpace {
    XToothService::XToothService(
        XToothEngine& engine,
        const string& name
    ) noexcept : m_engine(engine), m_autobus(make_unique<AutoBus>()) {
        _subscribe();
        _init(name);
    }

    XToothService::XToothService(XToothEngine& engine) noexcept
        : m_engine(engine) {}

    XToothService::~XToothService() {
        _reset();
        m_autobus.reset();
    }

    void XToothService::_init(const string& name) {
        _reset();
        m_name = name;
    }

    void XToothService::_reset() {
        m_arrow_ids.clear();
    }

    void XToothService::_subscribe() {
        // 拾取射线
        m_autobus->subscribe<void(const Point&, const Direction&)>(
            SignalPolicy::Sync, "render/picking_ray",
            bind(&XToothService::_pick_from_ray, this, placeholders::_1, placeholders::_2)
        );

        // 清除拾取射线
        m_autobus->subscribe<void()>(
            SignalPolicy::Sync, "render/clear_picking",
            bind(&XToothService::_clear_arrow, this)
        );
    }

    void XToothService::simulate() {
        auto _service = ContextHub::getInstance()->getService<shared_ptr<Drawable>(const string&)>("render/load_mesh");
        // auto mesh = _service.sync_invoke("static/models/Mikey.obj");
    }

    void XToothService::_pick_from_ray(const Point& ori, const Direction& dir) {
    }

    void XToothService::_clear_arrow() {
        // 通知渲染器 清除
        auto _service = ContextHub::getInstance()->getService<void(int)>("render/delete_mesh");
        for (auto& _id : m_arrow_ids) {
            _service.sync_invoke(_id);
        }
        m_arrow_ids.clear();
    }
}
