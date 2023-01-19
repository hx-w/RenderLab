#include "toolkit.h"
#include "tooth_pack.h"
#include "engine.h"
#include "service.h"

#include <iostream>
#include <vector>
#include <pybind11/embed.h>
#include <pybind11/numpy.h>
#include <geom_types.h>
#include <glm/gtc/type_ptr.hpp>

using namespace std;
namespace py = pybind11;

#define SERVICE_INST ToothEngine::get_instance()->get_service()


namespace ToothSpace {
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
			py::scoped_interpreter guard{};
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
			py::scoped_interpreter guard{};
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
			py::scoped_interpreter guard{};
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
		py::scoped_interpreter guard{};
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


	void action_node_1(shared_ptr<ToothPack> tpack) {
		auto& meshes_rec = tpack->get_meshes();
		auto& basedir = tpack->get_basedir();
		auto& ctx = tpack->get_context();

		// only calc one obb transf
		try {
			py::scoped_interpreter guard{};
			auto _py_os = py::module_::import("os");
			auto _py_trimesh = py::module_::import("trimesh");

			auto full_path = _py_os.attr("path").attr("join")(basedir, meshes_rec.begin()->first);

			auto msh_inst = _py_trimesh.attr("load")(full_path);

			// [Auto fix position]
			if (any_cast<bool>(ctx->node_states.at(NodeId_1).at("Auto fix position"))) {
				auto obb_transf = msh_inst.attr("apply_obb")();

				auto _mat = obb_transf.cast<py::array_t<float>>().unchecked<2>(); // 2-dim

				// must be 4x4 ( transposed )
				float flt_mat[16] = {
					_mat(0, 0), _mat(1, 0), _mat(2, 0), _mat(3, 0),
					_mat(0, 1), _mat(1, 1), _mat(2, 1), _mat(3, 1),
					_mat(0, 2), _mat(1, 2), _mat(2, 2), _mat(3, 2),
					_mat(0, 3), _mat(1, 3), _mat(2, 3), _mat(3, 3),
				};
				auto model_transf = glm::make_mat4(flt_mat);
				/// [slot] to renderer
				for (auto& msh : meshes_rec) {
					auto res = SERVICE_INST->slot_set_drawable_property(
						msh.second, "model_transf", model_transf
					);
				}
			}


		}
		catch (exception& e) {
			cout << e.what() << endl;
			return;
		}
	}
}

