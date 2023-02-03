#ifndef TOOTH_WORKFLOW_H
#define TOOTH_WORKFLOW_H

#include <memory>
#include <string>
#include <atomic>
#include <mutex>
#include <vector>
#include <geom_types.h>

namespace ToothSpace {
	class ToothPack;
	class Workspace {
	public:
		Workspace();
	
	public:
		void init_workspace();

		/// STEP-1
		void fetch_filepath(const std::string& /* filepath */, bool = false);


		/// STEP-2 confirm workflow
		///		since WkflowCtxPtr is a pointer, only flow_id is needed
		void confirm_workflow(int /* flow_id */, int /* status */);


		/// active workflow stage
		void active_stage(int /* flow_id */, int /* node_id */);


		/// handle picked points
		void pick_points_handler(
			std::vector<uint32_t>&,
			std::vector<geometry::Point3f>&,
			std::vector<geometry::Vector3f>&
		);

		/// handle picked vertex
		void pick_vertex_handler(uint32_t, uint32_t);


		/// compute nurbs reverse by recieved points
		void compute_nurbs_reverse(std::vector<std::vector<geometry::Point3f>>&, const std::pair<int, int>&);

		/// generate depth from recieved selected meshes id
		void generate_depth(const std::vector<uint32_t>&);

		void set_heatmap_style(const std::string&);

	private:
		int _gen_wkflow_id();

		std::shared_ptr<ToothPack> _find_tpack(int /* flow_id */);

	private:
		std::atomic<int> _curr_wkflow_id;

		// stacked tooth packs
		std::vector<std::shared_ptr<ToothPack>> _tooth_packs;

		std::string _heatmap_style = "jet";

		std::once_flag _inited;
	};
}

#endif