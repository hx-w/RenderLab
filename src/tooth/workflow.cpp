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

#include <tinynurbs/tinynurbs.h>

using namespace std;
using namespace RenderSpace;

#define SERVICE_INST ToothEngine::get_instance()->get_service()
#define TOOLKIT_EXEC(func, prefix, ...) \
			string log_msg = ""; \
			auto _code = static_cast<int>(func(__VA_ARGS__ log_msg)); \
			switch (_code) {                     \
			case 0: SERVICE_INST->slot_add_log("error", prefix" error, " + log_msg); break; \
			case 1: SERVICE_INST->slot_add_log("info", prefix" successfully, " + log_msg); break; \
			case 2: SERVICE_INST->slot_add_log("warn", prefix" suspended, " + log_msg); break;    \
			default: break;                      \
            }


namespace ToothSpace {
	Workspace::Workspace() {
		atomic_init(&_curr_wkflow_id, 1);
	}

	void Workspace::init_workspace() {
		call_once(_inited, []() {
			TOOLKIT_EXEC(init_workenv, "init workspace", )
		});
	}

	void Workspace::fetch_filepath(const string& filepath, bool force) {
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
			action_node_1(tpack, _heatmap_style);
			break;
		case NodeId_2:
			/// pick mode change
			action_node_2(tpack);
			// pmtr_nurbs
			break;
		case NodeId_3:
			action_node_3(tpack);
			// pmtr_remesh
			break;
		case NodeId_4:
			action_node_4(tpack);
			// generate_GT
			break;
		case NodeId_5:
			action_node_5(tpack);
			// generate_ML
			break;
		case NodeId_6:
			action_node_6(tpack);
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
		auto flow_id = SERVICE_INST->slot_get_current_flow_id();
		auto tpack = _find_tpack(flow_id);

		auto& nd_order = tpack->get_context()->node_order;
		// nurbs case
		if (find(nd_order.begin(), nd_order.end(), NodeId_2) != nd_order.end()) {
			// [DEBUG] show arrow
			auto picked_num = picked_ids.size();
			for (auto i = 0; i < picked_num; ++i) {
				auto ray = geometry::Ray(picked_pnts[i], picked_nmls[i]);
				auto arrow_id = SERVICE_INST->slot_show_arrow(ray, 0.5f, geometry::Vector3f(0.1f, 0.1f, 0.1f));
				SERVICE_INST->notify<void(uint32_t, geometry::Point3f&)>(
					"/add_nurbs_point_record", arrow_id, picked_pnts[i]
				);
			}
		}
	}

	void Workspace::pick_vertex_handler(uint32_t draw_id, uint32_t vertex_id, bool hover) {
		if (hover) {
			_hover_vertex_handler(draw_id, vertex_id);
		}
		else {
			_pick_vertex_handler(draw_id, vertex_id);
		}
	}

	void Workspace::compute_nurbs_reverse(vector<vector<geometry::Point3f>>& points_pack, const pair<int, int>& sample_rate) {
		if (points_pack.empty()) return;
		auto max_row = points_pack.size();
		auto max_col = (*points_pack.begin()).size();

		vector<geometry::Point3f> control_points;
		vector<vector<geometry::Point3f>> ctrl_matrix;
		vector<float> knots_u, knots_v;
		// u = row, v = col

		// [1] for all U
		for (auto row = 0; row < max_row; ++row) {
			vector<geometry::Point3f> _ctrl_pnts;

			compute_nurbs_curve_info(points_pack[row], _ctrl_pnts, knots_v);

			// append 
			ctrl_matrix.emplace_back(_ctrl_pnts);
		}

		// [2] for all V
		// add empty header and tail
		ctrl_matrix.insert(ctrl_matrix.begin(), vector<geometry::Vector3f>());
		ctrl_matrix.emplace_back(vector<geometry::Vector3f>());
		for (auto col = 0; col < max_col; ++col) {
			vector<geometry::Point3f> _pnts(max_row);
			for (auto row = 0; row < max_row; ++row) {
				_pnts[row] = points_pack[row][col];
			}

			vector<geometry::Point3f> _ctrl_pnts;
			compute_nurbs_curve_info(_pnts, _ctrl_pnts, knots_u);

			// add header and tail
			ctrl_matrix[0].emplace_back(_pnts[0]);
			ctrl_matrix.back().emplace_back(_pnts.back());
		}
		ctrl_matrix[0].insert(ctrl_matrix[0].begin(), ctrl_matrix[0][0]);
		ctrl_matrix[0].emplace_back(ctrl_matrix[0].back());
		ctrl_matrix.back().insert(ctrl_matrix.back().begin(), ctrl_matrix.back()[0]);
		ctrl_matrix.back().emplace_back(ctrl_matrix.back().back());

		// [3] flatten ctrl_matrix to control_points
		auto _ctrl_row = ctrl_matrix.size();
		auto _ctrl_col = ctrl_matrix[0].size();
		for (auto row = 0; row < _ctrl_row; ++row) {
			for (auto col = 0; col < _ctrl_col; ++col) {
				control_points.emplace_back(ctrl_matrix[row][col]);
			}
		}

		/// construct nurbs mesh
		tinynurbs::RationalSurface3f nurbs_surf;
		nurbs_surf.degree_u = 3, nurbs_surf.degree_v = 3;
		nurbs_surf.knots_u = knots_u;
		nurbs_surf.knots_v = knots_v;
		nurbs_surf.control_points = {
			max_row + 2, max_col + 2, control_points
		};
		nurbs_surf.weights = {
			max_row + 2, max_col + 2, vector<float>((max_row + 2) * (max_col + 2), 1.0f)
		};

		if (tinynurbs::surfaceIsValid(nurbs_surf)) {
			const auto& [row_num, col_num] = sample_rate;
			auto vertices = vector<geometry::Point3f>();
			auto faces = vector<geometry::Vector3u>();

			/**
			 *    x ------- x + 1
			 *    |     /     |
			 *    x+col -- x+col+1
			 */
			
			for (auto row = 0; row < row_num; ++row) {
				for (auto col = 0; col < col_num; ++col) {
					auto _u = static_cast<float>(row) / static_cast<float>(row_num);
					auto _v = static_cast<float>(col) / static_cast<float>(col_num);

					auto vert = tinynurbs::surfacePoint(nurbs_surf, _u, _v);
					vertices.emplace_back(vert);

					if (row != row_num - 1 && col != col_num - 1) {
						// update faces
						auto _x = row * col_num + col;
						faces.emplace_back(geometry::Vector3u(_x, _x + col_num, _x + 1));
						faces.emplace_back(geometry::Vector3u(_x + 1, _x + col_num, _x + col_num + 1));
					}
				}
			}

			auto mesh = geometry::Mesh(vertices, faces);

			auto mesh_id = SERVICE_INST->slot_add_mesh(mesh, { {"topo_shape", sample_rate} });

			// show in proj panel
			SERVICE_INST->notify<void(const string&, uint32_t)>("/register_mesh_to_current_proj", string("remeshed(nurbs)-") + to_string(mesh_id) + ".obj", mesh_id);


			/// current state NodeId_2
			/// go next state
			auto flow_id = SERVICE_INST->slot_get_current_flow_id();
			SERVICE_INST->notify<void(uint32_t)>("/finish_current_stage", flow_id);
		}
		else {
			// nurbs not valid
			SERVICE_INST->slot_add_log("error", "nurbs surface is invalid");
		}
	}

	void Workspace::compute_parameter_remesh(uint32_t draw_id) {
		uint32_t param_id = 0, remesh_id = 0;
	
		auto flow_id = SERVICE_INST->slot_get_current_flow_id();
		auto tpack = _find_tpack(flow_id);

		auto& nd_order = tpack->get_context()->node_order;
		// nurbs case
		if (find(nd_order.begin(), nd_order.end(), NodeId_3) != nd_order.end()) {
			auto U = any_cast<int>(tpack->get_context()->node_states[NodeId_3]["Remesh U"]);
			auto V = any_cast<int>(tpack->get_context()->node_states[NodeId_3]["Remesh V"]);
			_compute_parameter_remesh(draw_id, remesh_id, param_id, U, V);
	
			SERVICE_INST->notify<void(const string&, uint32_t)>(
				"/register_mesh_to_current_proj",
				string("remeshed(param)-") + to_string(remesh_id) + ".obj", remesh_id
			);
			SERVICE_INST->notify<void(const string&, uint32_t)>(
				"/register_mesh_to_current_proj",
				string("param_domain-") + to_string(remesh_id) + ".obj", param_id
			);

			// clear picked
			auto ext = MeshDrawableExtManager::get_mesh_ext(draw_id);
			ext->boundary_length = 0.f;
			for (auto& _p : ext->m_boundary_corners) {
				SERVICE_INST->slot_remove_drawable(_p.first);
			}
			ext->m_boundary_corners.clear();

			/// current state NodeId_3
			/// go next state
			auto flow_id = SERVICE_INST->slot_get_current_flow_id();
			SERVICE_INST->notify<void(uint32_t)>("/finish_current_stage", flow_id);
		}
	}

	void Workspace::generate_depth(const vector<uint32_t>& mshes_id) {
		auto flow_id = SERVICE_INST->slot_get_current_flow_id();
		auto tpack = _find_tpack(flow_id);

		vector<float> depth;
		if (tpack->get_context()->proj_t == Proj_CBCT) {
			if (mshes_id.size() != 4) return;
			compute_tooth_depth_GT(mshes_id, tpack, depth);

		}
		else if (tpack->get_context()->proj_t == Proj_IOS){
			if (mshes_id.size() != 2) return;
			/// [TODO]
		}
		else {
			assert(false);
		}

		/// [DEBUG] change color
		MeshDrawableExtManager::set_mesh_cache(mshes_id[0], "depth_GT", depth);
		MeshDrawableExtManager::switch_color_cache(mshes_id[0], "depth_GT", _heatmap_style);

		SERVICE_INST->notify<void(uint32_t)>("/finish_current_stage", flow_id);
	}

	void Workspace::set_heatmap_style(const string& _stl) {
		_heatmap_style = _stl;

		// change exist
		MeshDrawableExtManager::set_main_color(_stl);
	}
}
