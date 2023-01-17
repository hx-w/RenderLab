#pragma once

#include "base.h"

#include <memory>

namespace ToothSpace {
	class ToothPack;
}

namespace GUISpace {
	class ProjectPanel : GUIComponentBase {
	public:
		static void add_tooth_pack(std::shared_ptr<ToothSpace::ToothPack>);

		static void render();
	};
}
