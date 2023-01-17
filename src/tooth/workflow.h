#ifndef TOOTH_WORKFLOW_H
#define TOOTH_WORKFLOW_H

#include <memory>
#include <string>
#include <atomic>
#include <vector>

namespace ToothSpace {
	class ToothPack;
	class Workspace {
	public:
		Workspace();
	
	public:
		/// STEP-1
		void fetch_filepath(const std::string& /* filepath */, bool = false);


		/// STEP-2 confirm workflow
		///		since WkflowCtxPtr is a pointer, only flow_id is needed
		void confirm_workflow(int /* flow_id */, int /* status */);

	private:
		int _gen_wkflow_id();

	private:
		std::atomic<int> _curr_wkflow_id;

		// stacked tooth packs
		std::vector<std::shared_ptr<ToothPack>> _tooth_packs;
	};
}

#endif