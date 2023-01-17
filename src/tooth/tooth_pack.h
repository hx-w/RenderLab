#pragma once

#include <map>
#include <string>

#include "wkflow_context.h"

namespace ToothSpace {
	class ToothPack {
	public:
		ToothPack(int /* id */, const std::string& /* filepath */);

		WkflowCtxPtr get_context() { return wkflow_ctx;  }
		std::map<std::string, uint32_t>& get_meshes() { return meshes;  }

	private:
		WkflowCtxPtr wkflow_ctx;
		
		std::map<std::string, uint32_t> meshes; // <name, id>

		std::string basedir;
	};
}
