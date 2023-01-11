#ifndef TOOTH_WORKFLOW_H
#define TOOTH_WORKFLOW_H

#include <memory>
#include <string>

namespace ToothSpace {
	class Workspace {
	public:
		Workspace() = default;

	public:
		/// [Slots] GUI/filepath_selected
		void slot_fetch_filepath(const std::string&);
	};
}

#endif