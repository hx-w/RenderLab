#pragma once

#include <vector>
#include <string>
#include <memory>
#include <map>

namespace RenderSpace {
	class DrawableBase;
	class NewMeshDrawable;
}

namespace ToothSpace {
	class MeshDrawableExt {
	public:
		std::map<std::string, std::vector<float>> m_buffers; // {'mean_curvature': [...]}

		// vertices adj
		std::vector<std::vector<uint32_t>> m_vert_adj;
		
		// for parameter remesh
		std::vector<uint32_t> m_vert_boundary;
		std::vector<std::pair<uint32_t, uint32_t>> m_boundary_corners; // { draw_id: vertex_id }
		float boundary_length = 0.f;
		bool m_corner_order = true; // default order
	};

	class MeshDrawableExtManager {
	public:
		static void cache_mesh_ext(
			uint32_t, std::shared_ptr<RenderSpace::DrawableBase>, const std::string& /* type */,
			const std::string& /* heatmap style */ = ""
		);

		static void _cache_adj(uint32_t, std::shared_ptr<RenderSpace::NewMeshDrawable>);

		static void _cache_boundary(uint32_t, std::shared_ptr<RenderSpace::NewMeshDrawable>);

		static std::shared_ptr<MeshDrawableExt> get_mesh_ext(uint32_t);

		static void set_mesh_cache(uint32_t, const std::string&, std::vector<float>&);

		// change color by cache
		static void switch_color_cache(uint32_t, const std::string& /* type */, const std::string& /* style */);

		static void set_main_color(const std::string&);
	};
}

