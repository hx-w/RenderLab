#pragma once

#include "base.h"

#include <memory>
#include <glm/glm.hpp>
#include <string>

namespace ToothSpace {
	class ToothPack;
	class MeshDrawableExt;
}

namespace GUISpace {
	class ProjectPanel : GUIComponentBase {
	public:
		static void add_tooth_pack(std::shared_ptr<ToothSpace::ToothPack>);

		static void render();

		static void next_workflow_stage(int /* flow_id */);

		// nurbs picking
		static void add_picked_nurbs_points(uint32_t, glm::vec3&);

		// add nurbs surface
		static void register_mesh(const std::string&, uint32_t);

		// remesh(parameter) ext
		static void register_parameter_mesh_ext(uint32_t, std::shared_ptr<ToothSpace::MeshDrawableExt>);

		static uint32_t get_current_flow_id();
	};
}
