/*****************************************************************//**
 * \file   node_flow.cpp
 * \brief  !!!important
 * 
 *         ImNodes use only ONE context, 
 *         so dont use NodeId_X as shared
 * 
 *         solution: 
 *            RealNodeId_X for each flow: NodeId_X + flow_id
 *         since flow_id and NodeId_X both are hash value
 *         NodeAttrId is increase 1 each time for NodeId_X
 *  
 * \author Administrator
 * \date   January 2023
 *********************************************************************/


#include <iostream>
#include <cassert>
#include <mutex>
#include <imgui.h>
#include <imnodes.h>

#include "node_flow.h"

//#define SUBNODE(node_id, inc) ((std::hash<int>()(node_id) % 0xdeadbeef) + inc)
#define SUBNODE(node_id, inc) (node_id + inc)

#define SHOWNODE(node_id, title, deft_color, active_color, hover_color, ...) \
    ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32##deft_color); \
    ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32##active_color); \
    ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32##hover_color); \
    ImNodes::BeginNode(node_id); \
    ImNodes::BeginNodeTitleBar(); \
    ImGui::TextUnformatted(title); \
    ImNodes::EndNodeTitleBar(); \
    __VA_ARGS__ \
    ImNodes::EndNode(); \
    ImNodes::PopColorStyle(); \
    ImNodes::PopColorStyle(); \
    ImNodes::PopColorStyle();

#define HASHLINK(link) ((hash<int>()(link.first) + hash<int>()(link.second)) % 0xdeadbeef) 

using namespace std;
using namespace GUISpace;

static vector<unique_ptr<NodeFlow>> st_nodeflows; // all opened workflow editors

/**
 * Individual nodes.
 */
/// [Node] preprocess
void Node_Preprocess(int node_id) {
    SHOWNODE(node_id, "[Preprocess]",
        (0, 182, 208, 255), (0, 129, 201, 255), (0, 182, 248, 100),
        /* custom scripts */
        ImNodes::BeginOutputAttribute(SUBNODE(node_id, 1));
        ImGui::Text("output");
        ImNodes::EndOutputAttribute();     
    )
}

/// [Node] parameterization_nurbs
void Node_Pmtr_Nurbs(int node_id) {
    SHOWNODE(node_id, "[Parameter by Nurbs]",
        (119, 67, 219, 255), (59, 52, 134, 255), (119, 67, 219, 100),
        /* custom scripts */
        ImNodes::BeginInputAttribute(SUBNODE(node_id, 1));
        ImGui::Text("input");
        ImNodes::EndInputAttribute();

        ImNodes::BeginOutputAttribute(SUBNODE(node_id, 2));
        ImGui::Text("output");
        ImNodes::EndOutputAttribute();
    )
}

/// [Node] parameterization_remesh
void Node_Pmtr_Remesh(int node_id) {
    SHOWNODE(node_id, "[Parameter by Remesh]",
        (119, 67, 219, 255), (59, 52, 134, 255), (119, 67, 219, 100),
        /* custom scripts */
        ImNodes::BeginInputAttribute(SUBNODE(node_id, 1));
        ImGui::Text("input");
        ImNodes::EndInputAttribute();

        ImNodes::BeginOutputAttribute(SUBNODE(node_id, 2));
        ImGui::Text("output");
        ImNodes::EndOutputAttribute();
    )
}

/// [Node] generator_GT
void Node_Generator_GT(int node_id) {
    SHOWNODE(node_id, "[Generate by GT]",
        (5, 89, 91, 255), (6, 44, 48, 255), (5, 89, 91, 100),
        /* custom scripts */
        ImNodes::BeginInputAttribute(SUBNODE(node_id, 1));
        ImGui::Text("input");
        ImNodes::EndInputAttribute();

        ImNodes::BeginOutputAttribute(SUBNODE(node_id, 2));
        ImGui::Text("output");
        ImNodes::EndOutputAttribute();
    )
}

/// [Node] generator_ML
void Node_Generator_ML(int node_id) {
    SHOWNODE(node_id, "[Generate by ML]",
        (5, 89, 91, 255), (6, 44, 48, 255), (5, 89, 91, 100),
        /* custom scripts */
        ImNodes::BeginInputAttribute(SUBNODE(node_id, 1));
        ImGui::Text("input");
        ImNodes::EndInputAttribute();

        ImNodes::BeginOutputAttribute(SUBNODE(node_id, 2));
        ImGui::Text("output");
        ImNodes::EndOutputAttribute();
    )
}

/// [Node] postprocess
void Node_Postprocess(int node_id) {
    SHOWNODE(node_id, "[Postprecess]",
        (215, 82, 129, 255), (185, 49, 96, 255), (215, 82, 129, 100),
        /* custom scripts */
        ImNodes::BeginInputAttribute(SUBNODE(node_id, 1));
        ImGui::Text("input");
        ImNodes::EndInputAttribute();
    )
}

void NodeFlowHeaders() {
    {
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(3 / 7.0f, 0.6f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(3 / 7.0f, 0.7f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(3 / 7.0f, 0.8f, 0.8f));
		ImGui::Button("Active");
		ImGui::PopStyleColor(3);
    }
	ImGui::SameLine();
    {
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0 / 7.0f, 0.6f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0 / 7.0f, 0.7f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0 / 7.0f, 0.8f, 0.8f));
		ImGui::Button("Clear");
		ImGui::PopStyleColor(3);

    }
	ImGui::SameLine();
    {
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(5 / 7.0f, 0.6f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(5 / 7.0f, 0.7f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(5 / 7.0f, 0.8f, 0.8f));
		ImGui::Button("Check");
		ImGui::PopStyleColor(3);
    }
}

NodeFlow::NodeFlow(const WorkflowEntity& ent): m_flow_ent(ent) {
    /// set reverse node_attr_id to node_id
    node_attr_records[SUBNODE(NodeId_1, 1)] = NodeId_1;
    node_attr_records[SUBNODE(NodeId_2, 1)] = NodeId_2;
    node_attr_records[SUBNODE(NodeId_2, 2)] = NodeId_2;
    node_attr_records[SUBNODE(NodeId_3, 1)] = NodeId_3;
    node_attr_records[SUBNODE(NodeId_3, 2)] = NodeId_3;
    node_attr_records[SUBNODE(NodeId_4, 1)] = NodeId_4;
    node_attr_records[SUBNODE(NodeId_4, 2)] = NodeId_4;
    node_attr_records[SUBNODE(NodeId_5, 1)] = NodeId_5;
    node_attr_records[SUBNODE(NodeId_5, 2)] = NodeId_5;
    node_attr_records[SUBNODE(NodeId_6, 1)] = NodeId_6;
    m_ctx = ImNodes::EditorContextCreate();
}

NodeFlow::~NodeFlow() {
    ImNodes::EditorContextFree(m_ctx);
    m_ctx = nullptr;
}

void NodeFlow::reset_nodes_pos() {
    ImNodes::SetNodeGridSpacePos(NodeId_1, ImVec2(35.0f, 177.0f));
    ImNodes::SetNodeGridSpacePos(NodeId_2, ImVec2(202.0f, 278.0f));
    ImNodes::SetNodeGridSpacePos(NodeId_3, ImVec2(202.0f, 57.0f));
    ImNodes::SetNodeGridSpacePos(NodeId_4, ImVec2(476.0f, 72.0f));
    ImNodes::SetNodeGridSpacePos(NodeId_5, ImVec2(476.0f, 255.0f));
    ImNodes::SetNodeGridSpacePos(NodeId_6, ImVec2(691.0f, 177.0f));
}

void NodeFlow::delete_selected_links() {
    // in this context
    const int num_selected_links = ImNodes::NumSelectedLinks();
    if (num_selected_links > 0) {
        std::vector<int> selected_links(num_selected_links, 0);
        ImNodes::GetSelectedLinks(selected_links.data());
        
        for (auto lid : selected_links) {
            // carefully delete
            if (links.find(lid) != links.end()) {
                links.erase(lid);
            }
        }
    }
}

void NodeFlow::render() {
    /// A testcase
    ImGui::Begin(("Tooth Workflow Editor - " + m_flow_ent.flow_name).c_str());
    NodeFlowHeaders();

    ImNodes::BeginNodeEditor();
    // set to current context
    ImNodes::EditorContextSet(m_ctx);
    call_once(m_init, bind(&NodeFlow::reset_nodes_pos, this));

    const auto fid = m_flow_ent.flow_id * 10;
    Node_Preprocess(NodeId_1);
    Node_Pmtr_Nurbs(NodeId_2);
    Node_Pmtr_Remesh(NodeId_3);
    Node_Generator_GT(NodeId_4);
    Node_Generator_ML(NodeId_5);
    Node_Postprocess(NodeId_6);
    
    // create links for each frame
    for (auto& [_id, _link] : links) {
        ImNodes::Link(_id, _link.first, _link.second);
    }

    ImNodes::MiniMap(0.2, ImNodesMiniMapLocation_TopRight);
    ImNodes::EndNodeEditor();

    /**
     * ImGui::GetIO().WantCaptureKeyboard == true, use ImGui::IsKeyDown to capture
     * or set Want... = false, glfw will handle and notify from renderer
     * both are fine
     * !!!important
     */
    if (ImGui::IsKeyDown(ImGuiKey_Backspace) ||
        ImGui::IsKeyDown(ImGuiKey_Delete) ||
        ImGui::IsKeyDown(ImGuiKey_X)
    ) {
        delete_selected_links();
    }
    // add new link, render in next frame
    int start_attr = -1, end_attr = -1;
    if (ImNodes::IsLinkCreated(&start_attr, &end_attr)) {
        auto lp = LinkPair(start_attr, end_attr);
        links[HASHLINK(lp)] = lp;
    }
    ImGui::End();
}


/// ------------------- NodeFlowManager ------------------

void NodeFlowManager::open_workflow(
    int flow_id,
    const std::string& flow_name,
    std::shared_ptr<WorkflowParams> node_params
) {
    // check if flow_id is exists
    for (const auto& ndflow : st_nodeflows) {
        if (ndflow->get_flow_id() == flow_id) {
            /// [TODO] there must be a error
            assert(0, "Should not open a nodeflow while it is opened");
        }
    }

    // add to next frame
    st_nodeflows.emplace_back(
        make_unique<NodeFlow>(WorkflowEntity{flow_id, flow_name, node_params})
    );
}

void NodeFlowManager::check_valiation(int flow_id) {
    /// [TODO]
}

void NodeFlowManager::active(int flow_id) {
    /// [TODO]
}

void NodeFlowManager::delete_links() {
    for (auto& ndflow : st_nodeflows) {
        ndflow->delete_selected_links();
    }
}

void NodeFlowManager::render() {
    for (auto& ndflow : st_nodeflows) {
        ndflow->render();
    }
}
