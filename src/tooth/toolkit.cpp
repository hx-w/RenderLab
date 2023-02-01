#include "toolkit.h"
#include "tooth_pack.h"
#include "engine.h"
#include "service.h"
#include "drawable_ext.h"

#include <iostream>
#include <vector>
#include <pybind11/embed.h>
#include <pybind11/numpy.h>
#include <mesh.h>
#include <glm/gtc/type_ptr.hpp>
#include <geom_ext/drawable.h>


using namespace std;
using namespace RenderSpace;
namespace py = pybind11;

#define SERVICE_INST ToothEngine::get_instance()->get_service()

py::scoped_interpreter guard{};

using AABB = pair<float, float>;

namespace ToothSpace {
	template<class T>
	void vector_to_numpy(vector<glm::vec<3, T>>& vec, py::array& res) {
		const auto shape_1 = vec.size();
		const auto shape_2 = 3;
		auto raw_vec = new T[shape_1 * shape_2];
		for (auto _r = 0; _r < shape_1; ++_r) {
			for (auto _c = 0; _c < shape_2; ++_c) {
				raw_vec[_r * shape_2 + _c] = vec[_r][_c];
			}
		}
		res = py::array_t<T>(shape_1 * shape_2, raw_vec);
		delete[] raw_vec;
	}

	bool init_workenv(string& status) {
		/// init in this thread
		try {
			// init interpreter for current thread
			py::scoped_interpreter guard{};
			auto _py_pkg = py::module_::import(PY_INITENV_MODULE);
			auto reqs = py::make_tuple(PY_REQUIREMENTS);
			_py_pkg.attr("make_requirements_installed")(
				reqs, "https://pypi.tuna.tsinghua.edu.cn/simple"
			);
			status = "package loaded";
			return true;
		}
		catch (exception& e) {
			status = e.what();
			return false;
		}
	}

	int preprocess_tooth_path(const string& path, bool force, string& status) {
		try {
			//py::scoped_interpreter guard{};
			// check path is folder, and folder's elements valid
			auto _py_os = py::module_::import("os");
			if (!_py_os.attr("path").attr("isdir")(path).cast<bool>()) {
				status = "project target must be a directory";
				return 0;
			}

			// work with py script
			auto _py_pkg = py::module_::import(PY_LOADPROJ_MODULE);
			auto target_files = _py_pkg.attr("get_project_files")(path);
			if (target_files.cast<py::list>().empty()) {
				status = "not a valid project";
				return 0;
			}
			if (!_py_pkg.attr("update_config")(path, target_files, force).cast<bool>()) {
				status = "project source file changed";
				return 2;
			}
			else {
				status = "config updated";
				return 1;
			}
		}
		catch (exception& e) {
			cout << e.what() << endl;
		}
	}

	void get_tooth_pack_cache(ToothPack* tpack) {
		auto& meshes = tpack->get_meshes();	  // <string, uint32>
		auto& context = tpack->get_context();
		auto& path = tpack->get_basedir();
		try {
			//py::scoped_interpreter guard{};
			// load files names
			auto _py_os = py::module_::import("os");
			auto _py_toml = py::module_::import("toml");
			auto _py_pkg = py::module_::import(PY_LOADPROJ_MODULE);
			auto cfg_name = _py_pkg.attr("config_name"); // '.rdlab.toml'
			auto cfg_full = _py_os.attr("path").attr("join")(path, cfg_name);

			auto cfg_inst = _py_toml.attr("load")(cfg_full);
			auto _files = cfg_inst["source"]["files"].cast<py::list>();
			for (auto& _f : _files) {
				meshes[_f.cast<string>()] = -1; // default -1
			}

			// init context
			auto _ndorder = cfg_inst["workflow"]["node_order"].cast<py::list>();
			context->node_order.clear();
			for (auto _nd : _ndorder) {
				context->node_order.emplace_back(static_cast<NodeId>(_nd.cast<int>()));
			}
			auto _ctx = cfg_inst["workflow"]["context"].cast<py::dict>();

			/// [TODO] init context params
		}
		catch (exception& e) {
			cout << e.what() << endl;
		}
	}

	void save_tooth_pack_cache(ToothPack* tpack) {
		auto& context = tpack->get_context();
		auto& path = tpack->get_basedir();

		try {
			//py::scoped_interpreter guard{};
			// load files names
			auto _py_os = py::module_::import("os");
			auto _py_toml = py::module_::import("toml");
			auto _py_pkg = py::module_::import(PY_LOADPROJ_MODULE);
			auto cfg_name = _py_pkg.attr("config_name"); // '.rdlab.toml'
			auto cfg_full = _py_os.attr("path").attr("join")(path, cfg_name);

			auto cfg_inst = _py_toml.attr("load")(cfg_full);
			// clear node_order
			auto ndorder = py::list{};
			for (auto nd : context->node_order) {
				ndorder.append(static_cast<int>(nd));
			}
			cfg_inst["workflow"]["node_order"] = ndorder;
			/// [TODO] only save node_order for now


			/// save cfg_inst to cfg_full
			auto _opened_file = py::module_::import("builtins").attr("open")(cfg_full, "w");
			_py_toml.attr("dump")(cfg_inst, _opened_file);
			_opened_file.attr("close")();
		}
		catch (exception& e) {
			cout << e.what() << endl;
		}
	}

	void load_meshes_to_renderer(ToothPack* tpack) {
		//py::scoped_interpreter guard{};
		auto _py_os = py::module_::import("os");
		auto& meshes = tpack->get_meshes();
		auto& basedir = tpack->get_basedir();

		for (auto& [msh_name, msh_id] : meshes) {
			// msh_name has backword: xxx.obj
			auto full_path = _py_os.attr("path").attr("join")(basedir, msh_name).cast<string>();
			msh_id = SERVICE_INST->slot_load_mesh(full_path);
		}
	}

	void _topo_dfs(
		NodeId nd,
		map<NodeId, bool>& visited,
		const vector<LinkPair>& links,
		vector<NodeId>& node_order
	) {
		visited[nd] = true;
		// from nd as start
		for (const auto& lk : links) {
			if (lk.first == static_cast<int>(nd)) {
				auto nd_next = static_cast<NodeId>(lk.second);
				if (!visited.at(nd_next)) {
					_topo_dfs(nd_next, visited, links, node_order);
				}
			}
		}
		node_order.emplace_back(nd);
	}

	void topological_sort(const vector<LinkPair>& links, vector<NodeId>& node_order) {
		node_order.clear();
		// maintain nodes when add new node
		auto all_nodes = {
			NodeId_1, NodeId_2, NodeId_3, NodeId_4, NodeId_5, NodeId_6
		};

		map<NodeId, bool> visited;
		// init visited as false
		for (auto& nd : all_nodes) {
			visited[nd] = false;
		}

		auto search_start = [&](NodeId nd) {
			// in case of new added node except 6 nodes
			if (visited.find(nd) == visited.end()) visited[nd] = false;
			if (!visited.at(nd)) {
				_topo_dfs(nd, visited, links, node_order);
			}
		};

		for (auto iter = links.rbegin(); iter != links.rend(); ++iter) {
			search_start(static_cast<NodeId>((*iter).first));
			search_start(static_cast<NodeId>((*iter).second));
		}
		reverse(node_order.begin(), node_order.end());
	}


	void show_mesh_curvature(uint32_t draw_id, const string& type, const string& style) {
		auto inst = SERVICE_INST->slot_get_drawable_inst(draw_id);
		
		MeshDrawableExtManager::cache_mesh_ext(draw_id, inst, "curvature_mean", style);
	}

	void compute_nurbs_curve_info(
		vector<geometry::Point3f>& points,
		vector<geometry::Point3f>& control_points,
		vector<float>& knots
	) {
		control_points.clear();
		knots.clear();

		py::array py_points;
		vector_to_numpy(points, py_points);

		auto _py_pkg = py::module_::import(PY_NURBS_MODULE);

		auto k = 3;
		auto ret = _py_pkg.attr("compute_nurbs_reverse")(py_points, k).cast<py::tuple>();
		auto py_knots = ret[0].cast<py::array_t<float>>().unchecked<1>();
		auto py_ctrl_pnts = ret[1].cast<py::array_t<float>>().unchecked<2>();

		auto n = points.size();

		for (auto ind = 0; ind < n + k + 3; ++ind) {
			knots.emplace_back(py_knots(ind));
		}

		for (auto row = 0; row < n + 2; ++row) {
			control_points.emplace_back(
				geometry::Point3f(py_ctrl_pnts(row, 0), py_ctrl_pnts(row, 1), py_ctrl_pnts(row, 2))
			);
		}
	}

	void action_node_1(shared_ptr<ToothPack> tpack) {
		auto& meshes_rec = tpack->get_meshes();
		auto& basedir = tpack->get_basedir();
		auto& ctx = tpack->get_context();

		auto proj_aabb = geometry::default_bbox;

		for (auto& [_, id] : meshes_rec) {
			auto inst = SERVICE_INST->slot_get_drawable_inst(id);
			auto _aabb = dynamic_pointer_cast<NewMeshDrawable>(inst)->_aabb();

			if (_aabb.first.x < proj_aabb.first.x) proj_aabb.first.x = _aabb.first.x;
			if (_aabb.first.y < proj_aabb.first.y) proj_aabb.first.y = _aabb.first.y;
			if (_aabb.first.z < proj_aabb.first.z) proj_aabb.first.z = _aabb.first.z;
			if (_aabb.second.x > proj_aabb.second.x) proj_aabb.second.x = _aabb.second.x;
			if (_aabb.second.y > proj_aabb.second.y) proj_aabb.second.y = _aabb.second.y;
			if (_aabb.second.z > proj_aabb.second.z) proj_aabb.second.z = _aabb.second.z;
		}

		auto center = (proj_aabb.first + proj_aabb.second) / 2.f;

		// only calc one obb transf
		try {
			// [Auto fix position]
			if (any_cast<bool>(ctx->node_states.at(NodeId_1).at("Auto fix position"))) {
				auto model_transf = glm::mat4(
					1.f, 0.f, 0.f, 0.f,
					0.f, 1.f, 0.f, 0.f,
					0.f, 0.f, 1.f, 0.f,
					-center.x, -center.y, -center.z, 1.f
				);
				/// [SLOT] to renderer
				SERVICE_INST->slot_update_transform(model_transf);
			}


		}
		catch (exception& e) {
			cout << e.what() << endl;
			return;
		}

		/// [TODO]
		auto showed = false;
		for (auto& [_, msh_id] : meshes_rec) {
			if (!showed)
				show_mesh_curvature(msh_id, "mean", "viridis");
			else
				show_mesh_curvature(msh_id, "mean");
			showed = true;
		}
		
		SERVICE_INST->notify<void(uint32_t)>("/finish_current_stage", tpack->get_context()->flow_id);
	}
}

