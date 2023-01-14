#pragma once

#include <map>
#include <any>

namespace ToothSpace {
    using WorkflowParams = std::map<
        int /* node_id */,
        std::map<std::string /* option */, std::any> /* node_params */
    > ;
}
