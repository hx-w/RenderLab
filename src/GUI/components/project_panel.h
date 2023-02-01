#pragma once

#include "base.h"

#include <memory>
#include <glm/glm.hpp>
#include <string>

namespace ToothSpace {
	class ToothPack;
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
	};
}
