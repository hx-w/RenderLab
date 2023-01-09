#include "service.h"
#include "engine.h"
#include "viewer.h"

#include <memory>
#include <functional>
#include <communication/ContextHub.h>

namespace RenderSpace {
	class RenderWindowWidget;
}

using namespace std;
using namespace fundamental;
using namespace RenderSpace;


namespace GUISpace {
	GUIService::GUIService(GUIEngine& engine) noexcept:
		m_engine(engine), m_autobus(make_unique<AutoBus>()) {
		_subscribe_all();
	}

	GUIService::~GUIService() {
		m_autobus.reset();
	}

	void GUIService::_subscribe_all() {
		m_autobus->subscribe<void()>(SignalPolicy::Sync, "render/render_setup",
			bind(&GUISpace::ImGuiViewer::setup));
		m_autobus->subscribe<void(shared_ptr<RenderWindowWidget>)>(SignalPolicy::Sync, "render/pre_redraw",
			bind(&GUISpace::ImGuiViewer::update, placeholders::_1));
		m_autobus->subscribe<void()>(SignalPolicy::Sync, "render/render_destroy",
			bind(&GUISpace::ImGuiViewer::destroy));
	}

	/// [sync invoke] -> render/load_mesh
	void GUIService::slot_load_mesh(const string& file_path) {
		auto _service = ContextHub::getInstance()->getServiceTable<uint32_t(const string&)>();
		_service->sync_invoke("render/load_mesh", file_path);
	}
}