#include "workflow.h"

#include <iostream>
#include <cassert>
#include <line.hpp>
#include <mesh.h>
#include <geom_ext/drawable.h>

#include "toolkit.h"
#include "engine.h"
#include "service.h"
#include "tooth_pack.h"
#include "drawable_ext.h"

using namespace std;
using namespace RenderSpace;

#define SERVICE_INST ToothEngine::get_instance()->get_service()
#define TOOLKIT_EXEC(func, prefix, ...) \
			string log_msg = ""; \
			auto _code = static_cast<int>(func(__VA_ARGS__ log_msg)); \
			switch (_code) { \
			case 0: SERVICE_INST->slot_add_log("error", prefix##" error, " + log_msg); break; \
			case 1: SERVICE_INST->slot_add_log("info", prefix##" successfully, " + log_msg); break; \
			case 2: SERVICE_INST->slot_add_log("warn", prefix##" suspended, " + log_msg); break; \
			default: break; \
			}


namespace ToothSpace {
	Workspace::Workspace() {
		TOOLKIT_EXEC(init_workenv, "init workspace", )
		atomic_init(&_curr_wkflow_id, 1);
	}

	void Workspace::fetch_filepath(const string& filepath, bool force) {
		cout << filepath.c_str() << endl;
		TOOLKIT_EXEC(preprocess_tooth_path, "project load", filepath, force,)
		if (_code == 2) {
			SERVICE_INST->slot_add_notice(
				"Force to load the project?##" + filepath,
				"Files changed since the project builded\n\n"
				"Cache and config will be replaced if confirmed\n\n"
				"This cannot be undone!\n\n\n"
			);
			return;
		}
		if (_code != 1) return;
		// open workflow editor
		auto wkflow_id = _gen_wkflow_id();
		_tooth_packs.emplace_back(
			make_shared<ToothPack>(wkflow_id, filepath)
		);

		/// notice GUI module to setup
		SERVICE_INST->slot_open_workflow(_tooth_packs.back()->get_context());
	}

	/// status = 0, failed; = 1, success
	void Workspace::confirm_workflow(int flow_id, int status) {
		auto tpack = _find_tpack(flow_id);
		auto& flow_name = tpack->get_context()->flow_name;

		if (status == 0) {
			// failed case
			SERVICE_INST->slot_add_log("warn", "Discard workflow: " + flow_name);

			// do something
		}
		else if (status == 1) {
			// success case

			// save to cache
			save_tooth_pack_cache(tpack.get());

			SERVICE_INST->slot_add_log("info", "Confirm workflow: " + flow_name);
			// load mesh to renderer
			load_meshes_to_renderer(tpack.get());

			SERVICE_INST->slot_add_tooth_pack(tpack);
		}
		else {
			// invalid case
		}
	}

	int Workspace::_gen_wkflow_id() {
		return _curr_wkflow_id++;
	}

	shared_ptr<ToothPack> Workspace::_find_tpack(int flow_id) {
		for (auto& tpack : _tooth_packs) {
			if (tpack->get_context()->flow_id == flow_id) {
				return tpack;
			}
		}
		// failed
		return nullptr;
	}

	void Workspace::active_stage(int flow_id, int node_id) {
		auto tpack = _find_tpack(flow_id);
		auto node = static_cast<NodeId>(node_id);

		switch (node) {
		case NodeId_1:
			// preprocess
			action_node_1(tpack);
			break;
		case NodeId_2:
			// pmtr_nurbs
			break;
		case NodeId_3:
			// pmtr_remesh
			break;
		case NodeId_4:
			// generate_GT
			break;
		case NodeId_5:
			// generate_ML
			break;
		case NodeId_6:
			// postprocess
			break;
		default:
			// error
			assert(false);
		}

	}

	void Workspace::pick_points_handler(
		vector<uint32_t>& picked_ids,
		vector<geometry::Point3f>& picked_pnts,
		vector<geometry::Vector3f>& picked_nmls
	) {
		/// handler 
		

		// [DEBUG] show arrow
		auto picked_num = picked_ids.size();
		for (auto i = 0; i < picked_num; ++i) {
			auto ray = geometry::Ray(picked_pnts[i], picked_nmls[i]);
			SERVICE_INST->slot_show_arrow(ray, 0.5f, geometry::Vector3f(1.0f, 0.0f, 1.0f));
		}
	}

	void Workspace::pick_vertex_handler(uint32_t draw_id, uint32_t vertex_id) {
		static pair<uint32_t, uint32_t> st_last_hover_picked(-1, -1);
		// recover
		if (st_last_hover_picked.first != -1) {
			auto& [last_draw_id, last_vert_id] = st_last_hover_picked;

			auto last_draw_inst = SERVICE_INST->slot_get_drawable_inst(last_draw_id);
			auto last_mesh_inst = dynamic_pointer_cast<NewMeshDrawable>(last_draw_inst);
			auto& last_vertices = last_mesh_inst->_vertices();

			last_vertices[last_vert_id].Color = last_vertices[last_vert_id].BufColor;

			auto& last_vert_adj = MeshDrawableExtManager::get_mesh_ext(last_draw_id)->m_vert_adj;
			
			for (auto& adj : last_vert_adj[last_vert_id]) {
				last_vertices[adj].Color = last_vertices[adj].BufColor;
			}
			last_mesh_inst->get_ready();
		}

		st_last_hover_picked = make_pair(draw_id, vertex_id);
		if (draw_id == -1) {
			SERVICE_INST->slot_set_mouse_tooltip("");
			return; // end hover pick ( release CONTROL )
		}

		// show tooltip
		/// [TODO] only show curvature_mean
		auto& mesh_ext = MeshDrawableExtManager::get_mesh_ext(draw_id);
		if (mesh_ext->m_buffers.find("curvature_mean") == mesh_ext->m_buffers.end()) {
			SERVICE_INST->slot_set_mouse_tooltip("");
		}
		else {
			auto str_v = to_string(mesh_ext->m_buffers["curvature_mean"][vertex_id]);
			SERVICE_INST->slot_set_mouse_tooltip(
				"curv(mean): " + str_v
			);
		}

		auto draw_inst = SERVICE_INST->slot_get_drawable_inst(draw_id);
		auto mesh_inst = dynamic_pointer_cast<NewMeshDrawable>(draw_inst);
		auto& adjs = mesh_ext->m_vert_adj[vertex_id];

		auto& vertices = mesh_inst->_vertices();

		auto change_color = [&](uint32_t vid, glm::vec3&& clr) {
			vertices[vid].BufColor = vertices[vid].Color;
			vertices[vid].Color = clr;
		};
		// change adjs color to red

		change_color(vertex_id, glm::vec3(1.f, 0.5f, 0.f));
		for (auto& adj : adjs) {
			change_color(adj, glm::vec3(1.f, 0.f, 0.f));
		}

		mesh_inst->get_ready();
	}
}
