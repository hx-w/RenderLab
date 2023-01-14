#ifndef TOOTH_WORKFLOW_H
#define TOOTH_WORKFLOW_H

#include <memory>
#include <string>

namespace ToothSpace {
	class Workspace {
	public:
		Workspace();
	
	public:
		/// STEP-1
		void fetch_filepath(const std::string& /* filepath */, bool = false);

	};
}

#endif