/*****************************************************************//**
 * \file   node_flow.h
 * \brief  Tooth Workflow Editor using ImNodes
 * 
 * \author CarOL
 * \date   January 2023
 *********************************************************************/
#pragma once

#include "base.h"

#include <string>
#include <vector>
#include <map>
#include <any>
#include <memory>
#include <functional>

#include <wkflow_context.h>


struct ImNodesEditorContext;
namespace GUISpace {
    enum NodeId {
        NodeId_1 = 1 << 3   /* preprocess */,
        NodeId_2 = 1 << 5   /* pmtr_remesh */,
        NodeId_3 = 1 << 7   /* pmtr_nurbs */,
        NodeId_4 = 1 << 9   /* gen_GT */,
        NodeId_5 = 1 << 11  /* gen_ML */,
        NodeId_6 = 1 << 13  /* postprocess */,
    };


	/// mantain links
	using LinkPair = std::pair<int, int>;

    class NodeFlow {
    public:
        NodeFlow(ToothSpace::WkflowCtxPtr);

        ~NodeFlow();

        /// reset
        void reset_nodes_pos();

        /// action delete link
        /// @param type: int, 0 => selected, 1 => all
        void delete_links(int /* type */);

        void render();

        int get_flow_id() const { return m_flow_ctx->flow_id; };

        //void get_params_links(std::shared_ptr<WorkflowParams>, std::vector<LinkPair>&);

    private:
        ToothSpace::WkflowCtxPtr m_flow_ctx;

		std::map<int /* link_id */, LinkPair /* <node_attr_id, node_attr_id> */> links;
        std::map<int /* node_attr_id */, NodeId /* node_id */> node_attr_records;
        
        ImNodesEditorContext* m_ctx;

        std::once_flag m_init; // check if is first call, init nodes positions
    };


    class NodeFlowManager: public GUIComponentBase {
    public:
        /// recive raw, convert to WorkflowEntity, and create a NodeFlow
        static void open_workflow(
            ToothSpace::WkflowCtxPtr /* wkflow_ctx */
        );
        
		// check button callback
        static void check_valiation(int /* flow_id */, std::vector<NodeId>& /* exec_order */);

		// active button callback
        static void active(int /* flow_id */);

		// delete all selected links from all nodeflows
        static void delete_selected_links();
        static void delete_all_links(int /* flow_id */);
        
        static void render(); // render all
    };
}
