#include "service.h"
#include "engine.h"
#include "viewer.h"
#include "components/logger.h"
#include "components/modal_confirm.h"
#include "components/node_flow.h"
#include "components/project_panel.h"
#include "components/zmo.h"

#include <memory>
#include <functional>
#include <communication/ContextHub.h>
#include <wkflow_context.h>

namespace RenderSpace {
	class RenderWindowWidget;
}

namespace ToothSpace {
	class ToothPack;
	class MeshDrawableExt;
}

using namespace std;
using namespace fundamental;
using namespace RenderSpace;


namespace GUISpace {
	GUIService::GUIService(GUIEngine& engine) noexcept :
		m_engine(engine), m_autobus(make_unique<AutoBus>()) {
		_subscribe_all();
		_register_all();
	}

	GUIService::~GUIService() {
		m_autobus.reset();
	}

	void GUIService::_subscribe_all() {
		m_autobus->subscribe<void()>(SignalPolicy::Async, "render/render_setup",
			bind(&GUISpace::ImGuiViewer::setup));
		m_autobus->subscribe<void(shared_ptr<RenderWindowWidget>)>(SignalPolicy::Sync, "render/pre_redraw",
			bind(&GUISpace::ImGuiViewer::update, placeholders::_1));
		m_autobus->subscribe<void()>(SignalPolicy::Sync, "render/render_destroy",
			bind(&GUISpace::ImGuiViewer::destroy));
		m_autobus->subscribe<void(int)>(SignalPolicy::Sync, "render/keyboard_clicked",
			[this](int key) {
				if (key == 72  /* GLFW_KEY_H */) GUISpace::ImGuiViewer::change_visibility();
				if (key == 261 /* GLFW_KEY_DELETE */ || 
					key == 259 /* GLFW_KEY_BACKSPACE */)
					GUISpace::NodeFlowManager::delete_selected_links();
			});
		m_autobus->subscribe<void(uint32_t)>(SignalPolicy::Async, "tooth/finish_current_stage",
			bind(&GUISpace::ProjectPanel::next_workflow_stage, ::placeholders::_1));
		m_autobus->subscribe<void(uint32_t, glm::vec3&)>(SignalPolicy::Async, "tooth/add_nurbs_point_record",
			bind(&GUISpace::ProjectPanel::add_picked_nurbs_points, ::placeholders::_1, ::placeholders::_2));
		m_autobus->subscribe<void(const string&, uint32_t)>(SignalPolicy::Async, "tooth/register_mesh_to_current_proj",
			bind(&GUISpace::ProjectPanel::register_mesh, ::placeholders::_1, ::placeholders::_2));
		m_autobus->subscribe<void(int, int)>(SignalPolicy::Async, "render/window_resized",
			bind(&GUISpace::Zmo::set_big_window_size, ::placeholders::_1, ::placeholders::_2));
		m_autobus->subscribe<void(uint32_t, shared_ptr<ToothSpace::MeshDrawableExt>)>(SignalPolicy::Async, "tooth/register_mesh_ext",
			bind(&GUISpace::ProjectPanel::register_parameter_mesh_ext, ::placeholders::_1, ::placeholders::_2));
	}

	void GUIService::_register_all() {
		m_autobus->registerMethod<void(const string&, const string&)>(
			m_symbol + "/add_log",
			[this](const string& type, const string& msg) {
				LOG_LEVEL log_type = LOG_INFO;
				if (type == "info") log_type = LOG_INFO;
				else if (type == "error") log_type = LOG_ERROR;
				else if (type == "fatal") log_type = LOG_FATAL;
				else log_type = LOG_WARN;
				Logger::log(msg, log_type);
			}
		);

		m_autobus->registerMethod<void(const string&, const string&)>(
			m_symbol + "/add_notice",
			[this](const string& title, const string& detail) {
				GUISpace::ModalConfirm::add_notice(title, detail);
			}
		);

		m_autobus->registerMethod<void(const string&, function<void(any&&)>&&, any&&)>(
			m_symbol + "/add_notice",
			[this](const string& title, function<void(any&&)>&& custom, any&& param) {
				GUISpace::ModalConfirm::add_notice(
					title, 
					forward<decltype(custom)>(custom),
					forward<decltype(param)>(param)
				);
			}
		);

		m_autobus->registerMethod<void(ToothSpace::WkflowCtxPtr)>(
			m_symbol + "/open_workflow",
			bind(&GUISpace::NodeFlowManager::open_workflow, ::placeholders::_1)
		);

		m_autobus->registerMethod<void(shared_ptr<ToothSpace::ToothPack>)>(
			m_symbol + "/add_tooth_pack",
			bind(&GUISpace::ProjectPanel::add_tooth_pack, ::placeholders::_1)
		);

		m_autobus->registerMethod<void(const std::string&)>(
			m_symbol + "/set_mouse_tooltip",
			bind(&GUISpace::Zmo::set_mouse_tooltip, ::placeholders::_1)
			);
		
		m_autobus->registerMethod<uint32_t()>(
			m_symbol + "/get_current_flow_id",
			bind(&GUISpace::ProjectPanel::get_current_flow_id)
		);
	}

	/// [sync invoke] -> render/load_mesh
	void GUIService::slot_load_mesh(const string& file_path) {
		auto _service = ContextHub::getInstance()->getServiceTable<uint32_t(const string&)>();
		_service->sync_invoke("render/load_mesh", file_path);
	}

	shared_ptr<DrawableBase> GUIService::slot_get_drawable_inst(uint32_t msh_id) {
		auto _service = ContextHub::getInstance()->getServiceTable<shared_ptr<DrawableBase>(uint32_t)>();
		return _service->sync_invoke("render/get_drawable_inst", msh_id);
	}

	bool GUIService::slot_remove_drawable(uint32_t drawable_id) {
		auto _service = ContextHub::getInstance()->getServiceTable<bool(uint32_t)>();
		return _service->sync_invoke("render/remove_drawable", drawable_id);
	}

    bool GUIService::slot_set_drawable_property(uint32_t msh_id, const string& prop, const any& val) {
        auto _service = ContextHub::getInstance()->getServiceTable<bool(uint32_t, const string&, const any&)>();
        return _service->sync_invoke("render/set_drawable_property", msh_id, prop, val);
    }
}
