#pragma once

#include <map>
#include <any>
#include <vector>
#include <string>
#include <memory>

namespace ToothSpace {
    using WorkflowParams = std::map<
        int /* node_id */,
        std::map<std::string /* option */, std::any> /* node_params */
    > ;

    struct WorkflowContext {
        WorkflowContext(int fid, const std::string& fname):
            flow_id(fid), flow_name(fname), node_states(), node_order() {}

        int flow_id = 0;
        std::string flow_name = "";
        WorkflowParams node_states;
        std::vector<int> node_order;
    };

    using WkflowCtxPtr = std::shared_ptr<WorkflowContext>;
}
