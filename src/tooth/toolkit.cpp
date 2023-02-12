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
#include <line.hpp>

#include <thread>


using namespace std;
using namespace RenderSpace;
namespace py = pybind11;

#define SERVICE_INST ToothEngine::get_instance()->get_service()

using AABB = pair<float, float>;

namespace ToothSpace {
	py::scoped_interpreter py_guard{};
	py::gil_scoped_release py_release{};

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
			py::gil_scoped_acquire _acquire{};
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
			py::gil_scoped_acquire _acquire{};
			// check path is folder, and folder's elements valid
			auto _py_os = py::module_::import("os");
			if (!_py_os.attr("path").attr("isdir")(path).cast<bool>()) {
				status = "project target must be a directory";
				return 0;
			}

			auto x = py::module_::import("sys");
			py::print(x.attr("path"));

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
        return 0;
	}

	void get_tooth_pack_cache(ToothPack* tpack) {
		auto& meshes = tpack->get_meshes();	  // <string, uint32>
		auto& context = tpack->get_context();
		auto& path = tpack->get_basedir();
		try {
			py::gil_scoped_acquire py_acquire{};
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

			auto& nd_states = context->node_states;

			{
				auto node_1 = to_string(NodeId_1);
				nd_states[NodeId_1]["Ensure manifolds"] = _ctx[node_1.c_str()]["Ensure manifolds"].cast<bool>();
				nd_states[NodeId_1]["Auto fix position"] = _ctx[node_1.c_str()]["Auto fix position"].cast<bool>();
			}
			{
				auto node_2 = to_string(NodeId_2);
				nd_states[NodeId_2]["Samples U"] = _ctx[node_2.c_str()]["Samples U"].cast<int>();
				nd_states[NodeId_2]["Samples V"] = _ctx[node_2.c_str()]["Samples V"].cast<int>();
				nd_states[NodeId_2]["Assists"] = _ctx[node_2.c_str()]["Assists"].cast<bool>();
				nd_states[NodeId_2]["Remesh U"] = _ctx[node_2.c_str()]["Remesh U"].cast<int>();
				nd_states[NodeId_2]["Remesh V"] = _ctx[node_2.c_str()]["Remesh V"].cast<int>();
				nd_states[NodeId_2]["Weights"] = _ctx[node_2.c_str()]["Weights"].cast<string>();
			}
			{
				auto node_3 = to_string(NodeId_3);
				nd_states[NodeId_3]["Remesh U"] = _ctx[node_3.c_str()]["Remesh U"].cast<int>();
				nd_states[NodeId_3]["Remesh V"] = _ctx[node_3.c_str()]["Remesh V"].cast<int>();
			}


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
			py::gil_scoped_acquire py_acquire{};
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
			auto& nd_states = context->node_states;

			auto cfg_dict = py::dict{};
			// context saving
			{
				auto node_1 = to_string(NodeId_1);
				cfg_dict[node_1.c_str()] = py::dict{};
				cfg_dict[node_1.c_str()]["Ensure manifolds"] = any_cast<bool>(nd_states[NodeId_1]["Ensure manifolds"]);
				cfg_dict[node_1.c_str()]["Auto fix position"] = any_cast<bool>(nd_states[NodeId_1]["Auto fix position"]);
			}
			{
				auto node_2 = to_string(NodeId_2);
				cfg_dict[node_2.c_str()] = py::dict{};
				cfg_dict[node_2.c_str()]["Samples U"] = any_cast<int>(nd_states[NodeId_2]["Samples U"]);
				cfg_dict[node_2.c_str()]["Samples V"] = any_cast<int>(nd_states[NodeId_2]["Samples V"]);
				cfg_dict[node_2.c_str()]["Assists"] = any_cast<bool>(nd_states[NodeId_2]["Assists"]);
				cfg_dict[node_2.c_str()]["Remesh U"] = any_cast<int>(nd_states[NodeId_2]["Remesh U"]);
				cfg_dict[node_2.c_str()]["Remesh V"] = any_cast<int>(nd_states[NodeId_2]["Remesh V"]);
				cfg_dict[node_2.c_str()]["Weights"] = any_cast<string>(nd_states[NodeId_2]["Weights"]);
			}
			{
				auto node_3 = to_string(NodeId_3);
				cfg_dict[node_3.c_str()] = py::dict{};
				cfg_dict[node_3.c_str()]["Remesh U"] = any_cast<int>(nd_states[NodeId_3]["Remesh U"]);
				cfg_dict[node_3.c_str()]["Remesh V"] = any_cast<int>(nd_states[NodeId_3]["Remesh V"]);
			}
			/// [TODO] Other nodes


			cfg_inst["workflow"]["context"] = cfg_dict;

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
		py::gil_scoped_acquire py_acquire{};
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
		py::gil_scoped_acquire py_acquire{};
		control_points.clear();
		knots.clear();

		py::array py_points;
		vector_to_numpy(points, py_points);

		auto _py_pkg = py::module_::import(PY_NURBS_MODULE);

		auto k = 3;
		auto ret = _py_pkg.attr("compute_nurbs_reverse")(py_points, k).cast<py::tuple>();
		auto py_knots = ret[0].cast<py::array_t<double>>().unchecked<1>();
		auto py_ctrl_pnts = ret[1].cast<py::array_t<double>>().unchecked<2>();

		auto n = points.size();

		for (auto ind = 0; ind < n + k + 3; ++ind) {
			// ensure knots asc
			auto v = py_knots(ind);
			if (ind < n + k + 2 && v > py_knots(ind + 1))
				v = py_knots(ind + 1);
			knots.emplace_back(v);
		}

		for (auto row = 0; row < n + 2; ++row) {
			control_points.emplace_back(
				geometry::Point3f(py_ctrl_pnts(row, 0), py_ctrl_pnts(row, 1), py_ctrl_pnts(row, 2))
			);
		}
	}

	void _compute_parameter_remesh(uint32_t uns_id, uint32_t& str_id, uint32_t& param_id, int U, int V) {
		py::gil_scoped_acquire py_acquire{};

		auto draw_inst = SERVICE_INST->slot_get_drawable_inst(uns_id);
		if (draw_inst->_type() != GeomTypeMesh) return;
		auto msh = dynamic_pointer_cast<NewMeshDrawable>(draw_inst);

		auto ext = MeshDrawableExtManager::get_mesh_ext(uns_id);
		if (ext->boundary_length < 1e-6 || ext->m_boundary_corners.empty()) {
			return; // not a open mesh OR not select pivot
		}

		auto _py_pkg = py::module_::import(PY_PARAMETER_MODULE);

		py::array _verts, _faces;
		vector_to_numpy(msh->_raw()->vertices(), _verts);
		vector_to_numpy(msh->_raw()->faces(), _faces);

		auto res = _py_pkg.attr("parameter_remesh_cmd")(
			_verts, _faces, U, V, ext->m_boundary_corners[0].second, ext->m_corner_order
		).cast<py::tuple>();
		
		auto str_verts_num = res[0].attr("shape").cast<py::tuple>()[0].cast<int>();
		auto str_faces_num = res[1].attr("shape").cast<py::tuple>()[0].cast<int>();
		auto param_verts_num = res[2].attr("shape").cast<py::tuple>()[0].cast<int>();
		auto param_faces_num = res[3].attr("shape").cast<py::tuple>()[0].cast<int>();

		auto build_mesh_from_py = [](
			py::detail::tuple_accessor verts,
			py::detail::tuple_accessor faces,
			int num_v, int num_f
		) -> geometry::Mesh {
			auto _verts = verts.cast<py::array_t<double>>().unchecked<2>();
			auto _faces = faces.cast<py::array_t<long long int, py::array::c_style | py::array::forcecast>>().unchecked<2>();

			vector<geometry::Point3f> m_vertices(num_v);
			vector<geometry::Vector3u> m_faces(num_f);

			for (auto i = 0; i < num_v; ++i) {
				m_vertices[i] = geometry::Point3f(_verts(i, 0), _verts(i, 1), _verts(i, 2));
			}

			for (auto i = 0; i < num_f; ++i) {
				m_faces[i] = geometry::Vector3u(uint32_t(_faces(i, 0)), uint32_t(_faces(i, 1)), uint32_t(_faces(i, 2)));
			}
			
			return geometry::Mesh(m_vertices, m_faces);
		};

		auto str_msh = build_mesh_from_py(res[0], res[1], str_verts_num, str_faces_num);
		auto param_msh = build_mesh_from_py(
			res[2], res[3], param_verts_num, param_faces_num
		);

		str_id = SERVICE_INST->slot_add_mesh(str_msh, {{"topo_shape", pair<int, int>(U, V)}});
		param_id = SERVICE_INST->slot_add_mesh(param_msh);
	}

	void compute_tooth_depth_GT(
		const vector<uint32_t>& mshes_id,
		shared_ptr<ToothPack> tpack,
		vector<float>& depth
	) {
		py::gil_scoped_acquire py_acquire{};
		auto id_to_mesh = [](uint32_t id) -> shared_ptr<NewMeshDrawable> {
			auto draw_inst = SERVICE_INST->slot_get_drawable_inst(id);
			return dynamic_pointer_cast<NewMeshDrawable>(draw_inst);
		};

		vector<shared_ptr<NewMeshDrawable>> mshes;
		for (auto _id : mshes_id) {
			mshes.emplace_back(id_to_mesh(_id));
		}

		// init depth to all zero
		vector<float>(mshes[0]->_vertices().size(), 0.f).swap(depth);

		// topo shape for remeshed
		auto topo_shape = mshes[0]->topo_shape;

		if (topo_shape.first == 0 || topo_shape.first * topo_shape.second != depth.size()) {
			clog << "err!!! not a valid remeshed mesh" << endl;
			return;
		}

		auto _py_pkg = py::module_::import(PY_TOOTHDEPTH_MODULE);

		// construct params
		auto _py_args_1 = py::list{};
		for (auto msh : mshes) {
			py::array verts, faces;
			vector_to_numpy(msh->_raw()->vertices(), verts);
			vector_to_numpy(msh->_raw()->faces(), faces);

			_py_args_1.append(verts);
			_py_args_1.append(faces);
		}
		auto _py_args_2 = py::make_tuple(topo_shape.first, topo_shape.second);

		auto _py_depth = _py_pkg.attr("GeneratorGT").attr("generate_cmd")(
			_py_args_1, _py_args_2
		);

		auto _unchecked = _py_depth.cast<py::array_t<double>>().unchecked<1>();
		auto vsize = depth.size();
		for (auto ind = 0; ind < vsize; ++ind) {
			depth[ind] = _unchecked(ind);
		}
	}

	void compute_tooth_depth_ML(
		const vector<uint32_t>& mshes_id,
		shared_ptr<ToothPack> tpack,
		vector<float>& depth
	) {
		py::gil_scoped_acquire py_acquire{};
		auto id_to_mesh = [](uint32_t id) -> shared_ptr<NewMeshDrawable> {
			auto draw_inst = SERVICE_INST->slot_get_drawable_inst(id);
			return dynamic_pointer_cast<NewMeshDrawable>(draw_inst);
		};

		vector<shared_ptr<NewMeshDrawable>> mshes;
		for (auto _id : mshes_id) {
			mshes.emplace_back(id_to_mesh(_id));
		}

		// init depth to all zero
		vector<float>(mshes[0]->_vertices().size(), 0.f).swap(depth);


		/// [TODO]
	}

	void _hover_vertex_handler(uint32_t draw_id, uint32_t vertex_id) {
		// info shown case
		static pair<uint32_t, uint32_t> st_last_hover_picked(-1, -1);
		// recover
		//if (st_last_hover_picked.first != uint32_t(-1)) {
		//	auto& [last_draw_id, last_vert_id] = st_last_hover_picked;

		//	auto last_draw_inst = SERVICE_INST->slot_get_drawable_inst(last_draw_id);
		//	auto last_mesh_inst = dynamic_pointer_cast<NewMeshDrawable>(last_draw_inst);
		//	auto& last_vertices = last_mesh_inst->_vertices();

		//	auto& last_mesh_ext = MeshDrawableExtManager::get_mesh_ext(last_draw_id);
		//	if (last_mesh_ext != nullptr) {
		//		auto& last_vert_adj = last_mesh_ext->m_vert_adj;
		//		//last_vertices[last_vert_id].Color = last_vertices[last_vert_id].BufColor;
		//		
		//		//for (auto& adj : last_vert_adj[last_vert_id]) {
		//		//	last_vertices[adj].Color = last_vertices[adj].BufColor;
		//		//}
		//		//last_mesh_inst->get_ready();
		//	}
		//}

		st_last_hover_picked = make_pair(draw_id, vertex_id);
		if (draw_id == -1) {
			SERVICE_INST->slot_set_mouse_tooltip("");
			return; // end hover pick ( release CONTROL )
		}

		// show tooltip
		/// [TODO] only show curvature_mean
		auto mesh_ext = MeshDrawableExtManager::get_mesh_ext(draw_id);

		// if has depth, show depth
		if (mesh_ext == nullptr) {
			SERVICE_INST->slot_set_mouse_tooltip("");
			return;
		}
		if (mesh_ext->m_buffers.find("depth_GT") != mesh_ext->m_buffers.end()) {
			auto str_v = to_string(mesh_ext->m_buffers["depth_GT"][vertex_id]);
			SERVICE_INST->slot_set_mouse_tooltip(
				"depth: " + str_v
			);
		}
		else if (mesh_ext->m_buffers.find("curvature_mean") != mesh_ext->m_buffers.end()) {
			auto str_v = to_string(mesh_ext->m_buffers["curvature_mean"][vertex_id]);
			SERVICE_INST->slot_set_mouse_tooltip(
				"curv(mean): " + str_v
			);
		}
		else {
			SERVICE_INST->slot_set_mouse_tooltip("");
			return;
		}

		//auto draw_inst = SERVICE_INST->slot_get_drawable_inst(draw_id);
		//auto mesh_inst = dynamic_pointer_cast<NewMeshDrawable>(draw_inst);
		//auto& adjs = mesh_ext->m_vert_adj[vertex_id];

		//auto& vertices = mesh_inst->_vertices();

		//auto change_color = [&](uint32_t vid, glm::vec3&& clr) {
		//	vertices[vid].BufColor = vertices[vid].Color;
		//	vertices[vid].Color = clr;
		//};
		// change adjs color to red

		//change_color(vertex_id, glm::vec3(1.f, 0.5f, 0.f));
		//for (auto& adj : adjs) {
		//	change_color(adj, glm::vec3(1.f, 0.f, 0.f));
		//}

		//mesh_inst->get_ready();
	}

	void _pick_vertex_handler(uint32_t draw_id, uint32_t vertex_id) {
		auto draw_inst = SERVICE_INST->slot_get_drawable_inst(draw_id);
		auto mesh_inst = dynamic_pointer_cast<NewMeshDrawable>(draw_inst);
		auto ext = MeshDrawableExtManager::get_mesh_ext(draw_id);
		if (ext == nullptr) return;

		auto& bnd_verts = ext->m_vert_boundary;
		auto iter = find(bnd_verts.begin(), bnd_verts.end(), vertex_id);
		if (iter == bnd_verts.end() || ext->boundary_length < 1e-6) {
			// not a boundary vertex
			/// [Notify]
			SERVICE_INST->slot_add_log("warn", "not a boundary vertex");
			return;
		}
		
		// the index of picked vertex in boundary verts
		int pivot = iter - bnd_verts.begin(); 
		auto bnd_len = ext->boundary_length;

		// divide into 4 pieces
		auto piece_len = bnd_len / 4.f;
		auto bnd_size = bnd_verts.size();

		// clear legacy corner arrows
		for (auto& _pair: ext->m_boundary_corners) {
			auto [draw_id, vertex_id] = _pair;
			SERVICE_INST->slot_remove_drawable(draw_id);
		}
		ext->m_boundary_corners.clear();
		vector<uint32_t> corners;

		// set four corners
		auto order = ext->m_corner_order;
		auto& vertices = mesh_inst->_vertices();
		float summer = 0.f; // sum mer
		for (auto i = 0; i < bnd_size; ++i) {
			if (i == 0) {
				corners.emplace_back(bnd_verts[pivot % bnd_size]);
				continue;
			}

			auto ratio_old = floorf(summer / piece_len);
			if (order) {
				summer += glm::distance(
					vertices[bnd_verts[(pivot + i - 1) % bnd_size]].Position,
					vertices[bnd_verts[(pivot + i) % bnd_size]].Position
				);
			}
			else {
				summer += glm::distance(
					vertices[bnd_verts[(pivot - i + 1 + bnd_size) % bnd_size]].Position,
					vertices[bnd_verts[(pivot - i + bnd_size) % bnd_size]].Position
				);
			}

			auto ratio_new = floorf(summer / piece_len);
			
			if (ratio_old != ratio_new) {
				if (order) {
					corners.emplace_back(bnd_verts[(pivot + i) % bnd_size]);
				}
				else {
					corners.emplace_back(bnd_verts[(pivot - i + bnd_size) % bnd_size]);
				}
				if (corners.size() == 4) break; // full
			}
		}

		// draw arrows for corner
		for (auto cor_ind : corners) {
			// normal direction
			auto ray = geometry::Ray(vertices[cor_ind].Position, vertices[cor_ind].Normal);
			auto id = SERVICE_INST->slot_show_arrow(ray, 0.5f, glm::vec3(1.f, 0.f, 0.f));
			ext->m_boundary_corners.emplace_back(make_pair(id, cor_ind));
		}

		// register to project panel only if pivot picked
		SERVICE_INST->notify<void(uint32_t, shared_ptr<MeshDrawableExt>)>("/register_mesh_ext", draw_id, ext);
	}

	void action_node_1(shared_ptr<ToothPack> tpack, const string& style) {
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
				show_mesh_curvature(msh_id, "mean", style);
			else
				show_mesh_curvature(msh_id, "mean");
			showed = true;
		}
		
		SERVICE_INST->notify<void(uint32_t)>("/finish_current_stage", tpack->get_context()->flow_id);
	}

	void action_node_2(shared_ptr<ToothPack> tpack) {
		// change interact mode
		auto mode = (1 << 1) | (1 << 3); // ClickPointPick | HoverVertexPick
		SERVICE_INST->slot_set_interact_mode(mode);
	}

	void action_node_3(shared_ptr<ToothPack> tpack) {
		// change interact mode
		auto mode = 1 << 2; // ClickVertexPick
		SERVICE_INST->slot_set_interact_mode(mode);
	}

	void action_node_4(shared_ptr<ToothPack> tpack) {
		auto mode = (1 << 2) | (1 << 3); // ClickVertexPick
		SERVICE_INST->slot_set_interact_mode(mode);
	}

	void action_node_5(shared_ptr<ToothPack> tpack) {
	}

	void action_node_6(shared_ptr<ToothPack> tpack) {
	}
}

