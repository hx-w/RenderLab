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
    using WorkflowParams = std::map<
        int /* node_id */,
        std::map<std::string /* option */, std::any> /* node_params */
    >;

    struct WorkflowEntity {
        int flow_id;
        std::string flow_name;
        std::shared_ptr<WorkflowParams> node_params;
    };

    class NodeFlow {
    public:
        NodeFlow(const WorkflowEntity&);

        ~NodeFlow();

        /// reset
        void reset_nodes_pos();

        /// action delete link
        /// @param type: int, 0 => selected, 1 => all
        void delete_links(int /* type */);

        void render();

        int get_flow_id() const { return m_flow_ent.flow_id; };

        void get_params_links(std::shared_ptr<WorkflowParams>, std::vector<LinkPair>&);

    private:
        WorkflowEntity m_flow_ent;

		std::map<int /* link_id */, LinkPair /* <node_attr_id, node_attr_id> */> links;
        std::map<int /* node_attr_id */, NodeId /* node_id */> node_attr_records;
        
        ImNodesEditorContext* m_ctx;

        std::once_flag m_init; // check if is first call, init nodes positions
    };


    class NodeFlowManager: public GUIComponentBase {
    public:
        /// recive raw, convert to WorkflowEntity, and create a NodeFlow
        static void open_workflow(
            int /* flow_id */,
            const std::string& /* flow_name */,
            std::shared_ptr<WorkflowParams> /* node_params */
        );

        static void check_valiation(int /* flow_id */); // check button callback

        static void active(int /* flow_id */); // active button callback

        static void delete_selected_links(); // delete all selected links from all nodeflows
        static void delete_all_links(int /* flow_id */);
        
        static void render(); // render all
    };
}
