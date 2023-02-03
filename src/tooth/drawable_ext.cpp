#include "drawable_ext.h"
#include "toolkit.h"
#include <geom_ext/drawable.h>
#include <mesh.h>
#include <pybind11/embed.h>
#include <pybind11/numpy.h>
#include <glm/glm.hpp>
#include "service.h"
#include "engine.h"

using namespace std;
using namespace RenderSpace;
namespace py = pybind11;


namespace ToothSpace {
	#define SERVICE_INST ToothEngine::get_instance()->get_service()

	static map<uint32_t, shared_ptr<MeshDrawableExt>> st_mesh_exts;

	static map<uint32_t, pair<string /* type */, string /* style */>> st_visible_buffer; // each mesh has one visible buffer


	/// @date 2023.01.25
	/// use 1-dim cpp array to restore vector,
	/// and convert to py::array_t<T> type
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

	void MeshDrawableExtManager::_cache_adj(uint32_t id) {
		if (st_mesh_exts.find(id) == st_mesh_exts.end()) {
			st_mesh_exts[id] = make_shared<MeshDrawableExt>();
		}
		auto draw_ptr = SERVICE_INST->slot_get_drawable_inst(id);
		auto msh_ptr = dynamic_pointer_cast<NewMeshDrawable>(draw_ptr);

		/// compute adj
		const auto& vertices = msh_ptr->_raw()->get_vertices();
		const auto& faces = msh_ptr->_raw()->get_faces();

		auto& ext = st_mesh_exts[id];
		ext->m_vert_adj.clear();
		auto& adj = ext->m_vert_adj;

		adj.resize(vertices.size());
		for (auto& face : faces) {
			for (auto i = 0; i < 3; ++i) {
				if (std::find(adj[face[i]].begin(), adj[face[i]].end(), face[(i + 1) % 3]) == adj[face[i]].end())
					adj[face[i]].emplace_back(face[(i + 1) % 3]);
				if (std::find(adj[face[i]].begin(), adj[face[i]].end(), face[(i + 2) % 3]) == adj[face[i]].end())
					adj[face[i]].emplace_back(face[(i + 2) % 3]);
			}
		}
	}

	void MeshDrawableExtManager::cache_mesh_ext(
		uint32_t id,
		shared_ptr<DrawableBase> draw_ptr,
		const string& type,
		const string& heatmap_style
	) {
		static auto support_type = { "curvature_mean", "curvature_gaussian" };

		if (draw_ptr == nullptr || draw_ptr->_type() != GeomTypeMesh) return;
		if (std::find(support_type.begin(), support_type.end(), type) == support_type.end()) {
			return;
		}
		auto msh_ptr = dynamic_pointer_cast<NewMeshDrawable>(draw_ptr);

		auto ext = (
			st_mesh_exts.find(id) == st_mesh_exts.end() ?
			make_shared<MeshDrawableExt>() :
			st_mesh_exts.at(id)
		);

		/// only for curvature
		
		auto _py_pkg = py::module_::import(PY_PALETTE_MODULE);

		py::array py_verts, py_faces;
		vector_to_numpy(msh_ptr->_raw()->vertices(), py_verts);
		vector_to_numpy(msh_ptr->_raw()->faces(), py_faces);

		auto py_curv = _py_pkg.attr("py_compute_mesh_curvature")(
			py_verts, py_faces, (type == "curvature_mean" ? "mean" : "gaussian")
		);

		auto _curv = py_curv.cast<py::array_t<float>>().unchecked<1>();

		auto vec = vector<float>(msh_ptr->_vertices().size());
		
		for (auto ind = 0; ind < vec.size(); ++ind) {
			vec[ind] = _curv(ind);
		}

		ext->m_buffers[type].clear();
		ext->m_buffers[type].assign(vec.begin(), vec.end());
		st_mesh_exts[id] = ext;

		_cache_adj(id);

		// change color ?
		switch_color_cache(id, "curvature_mean", heatmap_style);
		//if (heatmap_style.length() > 0) {
		//	auto py_clr = _py_pkg.attr("py_mesh_palette")(py_curv, "viridis");
		//	/// convert numpy.ndarray to py::array_t<T>, and uncheck it
		//	auto clrs = py_clr.cast<py::array_t<float>>().unchecked<2>();

		//	auto vert_size = msh_ptr->_raw()->vertices().size();

		//	auto& vert_prims = msh_ptr->_vertices();

		//	for (auto ind = 0; ind < vert_size; ++ind) {
		//		vert_prims[ind].Color = glm::vec3(clrs(ind, 0), clrs(ind, 1), clrs(ind, 2));
		//	}
		//	msh_ptr->get_ready();
		//}

	}

	shared_ptr<MeshDrawableExt>
	MeshDrawableExtManager::get_mesh_ext(uint32_t id) {
		if (st_mesh_exts.find(id) == st_mesh_exts.end()) return nullptr;
		return st_mesh_exts.at(id);
	}

	void MeshDrawableExtManager::set_mesh_cache(uint32_t id, const string& key, vector<float>& cache) {
		if (st_mesh_exts.find(id) == st_mesh_exts.end()) {
			st_mesh_exts[id] = make_shared<MeshDrawableExt>();
			_cache_adj(id);
		}
		st_mesh_exts[id]->m_buffers[key].assign(cache.begin(), cache.end());
	}

	void MeshDrawableExtManager::switch_color_cache(uint32_t id, const string& type, const string& style) {
		if (st_mesh_exts.find(id) == st_mesh_exts.end()) return;
		if (st_mesh_exts[id]->m_buffers.find(type) == st_mesh_exts[id]->m_buffers.end()) return;

		auto draw_ptr = SERVICE_INST->slot_get_drawable_inst(id);
		if (draw_ptr == nullptr || draw_ptr->_type() != GeomTypeMesh) return;
		auto msh_ptr = dynamic_pointer_cast<NewMeshDrawable>(draw_ptr);
		
		auto _py_pkg = py::module_::import(PY_PALETTE_MODULE);

		auto& cache = st_mesh_exts[id]->m_buffers[type];
		auto _py_cache = py::array_t<float>(cache.size(), cache.data());

		auto py_clr = _py_pkg.attr("py_mesh_palette")(_py_cache, style);
		/// convert numpy.ndarray to py::array_t<T>, and uncheck it
		auto clrs = py_clr.cast<py::array_t<float>>().unchecked<2>();

		auto vert_size = msh_ptr->_raw()->vertices().size();

		auto& vert_prims = msh_ptr->_vertices();

		for (auto ind = 0; ind < vert_size; ++ind) {
			vert_prims[ind].Color = glm::vec3(clrs(ind, 0), clrs(ind, 1), clrs(ind, 2));
		}
		msh_ptr->get_ready();

		st_visible_buffer[id] = make_pair(type, style);
	}

	void MeshDrawableExtManager::set_main_color(const string& style) {
		// change all visible buffer to style
		for (auto& [_id, _pair] : st_visible_buffer) {
			auto& [_type, _stl] = _pair;
			if (style == _stl) continue;

			switch_color_cache(_id, _type, style);
		}
	}
}
