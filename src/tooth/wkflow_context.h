#pragma once

#include <map>
#include <any>
#include <vector>
#include <string>
#include <memory>

/// mantain links
using LinkPair = std::pair<int, int>;
enum NodeId {
	NodeId_1 = 1 << 3   /* preprocess */,
	NodeId_2 = 1 << 5   /* pmtr_nurbs */,
	NodeId_3 = 1 << 7   /* pmtr_remesh */,
	NodeId_4 = 1 << 9   /* gen_GT */,
	NodeId_5 = 1 << 11  /* gen_ML */,
	NodeId_6 = 1 << 13  /* postprocess */,
};

enum ProjectType {
	Proj_CBCT,  // 4 meshes
	Proj_IOS    // 2 meshes
};

namespace ToothSpace {
	using WorkflowParams = std::map<
		NodeId /* node_id */,
		std::map<std::string /* option */, std::any> /* node_params */
	>;

    struct WorkflowContext {
        WorkflowContext(int fid, const std::string& fname):
            flow_id(fid), flow_name(fname), node_states(), node_order() {
			// template states
			node_states[NodeId_1] = {
				{"Ensure manifolds", true}, // input manifolds ensure
				{"Auto fix position", true},
			};
			node_states[NodeId_2] = {
				{"CtrPtr size", std::make_pair(3, 5)},
				{"Assists", false},
				{"CtrPtr weights", "auto"},
				{"Remesh size", std::make_pair(100, 100)}
			};
			node_states[NodeId_3] = {
				{"Parameterize method", "Yenh 2019"},
				{"Remesh size", std::make_pair(100, 100)}
			};
			node_states[NodeId_4] = {

			};
			node_states[NodeId_5] = {

			};
			node_states[NodeId_6] = {

			};
		}

		int flow_id = 0;
        std::string flow_name = "";
        WorkflowParams node_states;

		// node_order manage
        std::vector<NodeId> node_order;
		int stage_curr = -1;

		// extra members
		ProjectType proj_t = Proj_CBCT; // reset in tooth pack constructor

    };

    using WkflowCtxPtr = std::shared_ptr<WorkflowContext>;
}
