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
        std::vector<NodeId> node_order;

		// extra members
    };

    using WkflowCtxPtr = std::shared_ptr<WorkflowContext>;
}
