#pragma once

#include <vector>
#include <string>
#include <memory>
#include <map>

namespace RenderSpace {
	class DrawableBase;
}

namespace ToothSpace {
	class MeshDrawableExt {
	public:
		std::map<std::string, std::vector<float>> m_buffers; // {'mean_curvature': [...]}

		// vertices adj
		std::vector<std::vector<uint32_t>> m_vert_adj;
	};

	class MeshDrawableExtManager {
	public:
		static void cache_mesh_ext(
			uint32_t, std::shared_ptr<RenderSpace::DrawableBase>, const std::string& /* type */,
			const std::string& /* heatmap style */ = ""
		);

		static std::shared_ptr<MeshDrawableExt> get_mesh_ext(uint32_t);
	};
}

