/*****************************************************************//**
 * \file   node_flow.cpp
 * \brief  !!!important
 * 
 *         'NodeFlow' maintain WkflowCtxPtr for each nodeflow
 *         'NodeFlowManager' is a collection of static functions for NodeFlow
 * 
 * \author CarOL
 * \date   January 2023
 *********************************************************************/


#include <iostream>
#include <cassert>
#include <mutex>
#include <algorithm>
#include <imgui.h>
#include <imnodes.h>

#include "node_flow.h"
#include "modal_confirm.h"

#include "../service.h"
#include "../engine.h"

// tooth toolkit
#include <toolkit.h>

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

#define SERVICE_INST GUIEngine::get_instance()->get_service()

using namespace std;
using namespace GUISpace;
using namespace ToothSpace;


static vector<unique_ptr<NodeFlow>> st_nodeflows; // all opened workflow editors

static void HelpMarker(const char* desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void ColoredButton(ImVec4&& clr, const char* title) {
    //ImGui::PushStyleColor(ImGuiCol_Button, forward<ImVec4>(clr));
    ImGui::Button(title);
    //ImGui::PopStyleColor();
}

/**
 * Individual nodes.
 */
/// [Node] preprocess
void Node_Preprocess(NodeId node_id, WkflowCtxPtr wkflow_ctx) {
    SHOWNODE(node_id, "[Preprocess]",
        (0, 182, 208, 255), (0, 129, 201, 255), (0, 182, 248, 100),
        /* custom scripts */
        ImNodes::BeginOutputAttribute(SUBNODE(node_id, 2));
        ImNodes::EndOutputAttribute();
        {
            // stupid, but works
			ImGui::Checkbox(
                "Ensure manifolds",
                (bool *)(& wkflow_ctx->node_states[NodeId_1]["Ensure manifolds"])
            );
            ImGui::SameLine();
            HelpMarker(
                "If input meshes are not manifolds,\n"
                "program will convert them to manifolds as possible"
            );
            ImGui::Checkbox(
                "Auto fix position",
                (bool*)(&wkflow_ctx->node_states[NodeId_1]["Auto fix position"])
            );
            ImGui::SameLine();
            HelpMarker(
                "Auto transform project to world origin"
            );
        }
    )
}

/// [Node] parameterization_nurbs
void Node_Pmtr_Nurbs(NodeId node_id, WkflowCtxPtr wkflow_ctx) {
    SHOWNODE(node_id, "[Parameter by Nurbs]",
        (119, 67, 219, 255), (59, 52, 134, 255), (119, 67, 219, 100),
        /* custom scripts */
        ImNodes::BeginInputAttribute(SUBNODE(node_id, 1));
        ImNodes::EndInputAttribute();
        ImGui::SameLine();
        ImNodes::BeginOutputAttribute(SUBNODE(node_id, 2));
        ImNodes::EndOutputAttribute();
    )
}

/// [Node] parameterization_remesh
void Node_Pmtr_Remesh(NodeId node_id, WkflowCtxPtr wkflow_ctx) {
    SHOWNODE(node_id, "[Parameter by Remesh]",
        (119, 67, 219, 255), (59, 52, 134, 255), (119, 67, 219, 100),
        /* custom scripts */
        ImNodes::BeginInputAttribute(SUBNODE(node_id, 1));
        ImNodes::EndInputAttribute();
        ImGui::SameLine();
        ImNodes::BeginOutputAttribute(SUBNODE(node_id, 2));
        ImNodes::EndOutputAttribute();
    )
}

/// [Node] generator_GT
void Node_Generator_GT(NodeId node_id, WkflowCtxPtr wkflow_ctx) {
    SHOWNODE(node_id, "[Generate by GT]",
        (5, 89, 91, 255), (6, 44, 48, 255), (5, 89, 91, 100),
        /* custom scripts */
        ImNodes::BeginInputAttribute(SUBNODE(node_id, 1));
        ImNodes::EndInputAttribute();
        ImGui::SameLine();
        ImNodes::BeginOutputAttribute(SUBNODE(node_id, 2));
        ImNodes::EndOutputAttribute();
    )
}

/// [Node] generator_ML
void Node_Generator_ML(NodeId node_id, WkflowCtxPtr wkflow_ctx) {
    SHOWNODE(node_id, "[Generate by ML]",
        (5, 89, 91, 255), (6, 44, 48, 255), (5, 89, 91, 100),
        /* custom scripts */
        ImNodes::BeginInputAttribute(SUBNODE(node_id, 1));
        ImNodes::EndInputAttribute();
        ImGui::SameLine();
        ImNodes::BeginOutputAttribute(SUBNODE(node_id, 2));
        ImNodes::EndOutputAttribute();
    )
}

/// [Node] postprocess
void Node_Postprocess(NodeId node_id, WkflowCtxPtr wkflow_ctx) {
    SHOWNODE(node_id, "[Postprecess]",
        (215, 82, 129, 255), (185, 49, 96, 255), (215, 82, 129, 100),
        /* custom scripts */
        ImNodes::BeginInputAttribute(SUBNODE(node_id, 1));
        ImNodes::EndInputAttribute();
    )
}

void NodeFlowHeaders(int flow_id) {
    /// =============== ACTIVE ====================
    {
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(3 / 7.0f, 0.6f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(3 / 7.0f, 0.7f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(3 / 7.0f, 0.8f, 0.8f));
        if (ImGui::Button("Active")) {
            NodeFlowManager::active(flow_id);
        }
		ImGui::PopStyleColor(3);
    }
	ImGui::SameLine();
    /// =============== CLEAR ====================
    {
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0 / 7.0f, 0.6f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0 / 7.0f, 0.7f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0 / 7.0f, 0.8f, 0.8f));
        if (ImGui::Button("Clear")) {
            NodeFlowManager::delete_all_links(flow_id);
        }
		ImGui::PopStyleColor(3);

    }
	ImGui::SameLine();
    /// =============== CHECK ====================
    {
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(5 / 7.0f, 0.6f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(5 / 7.0f, 0.7f, 0.7f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(5 / 7.0f, 0.8f, 0.8f));
		vector<NodeId> node_order;
        auto str_id_success = "Check successfully - " + to_string(flow_id);
        auto str_id_failed = "Check failed - " + to_string(flow_id);
        if (ImGui::Button("Check")) {
            /// [TODO]
            if (NodeFlowManager::check_valiation(flow_id, node_order)) {
                // success
            }
            else {
                // failed case
                ModalConfirm::add_notice("Checking Failed", [](any&& param) {
					ImGui::TextColored(ImVec4(10, 200, 10, 255), "node links is invalid!");
                    // a tricky style
                    auto ndlist = any_cast<vector<NodeId>>(param);
                    auto ndlist_sz = ndlist.size();
                    for (auto ind = 0; ind < ndlist_sz; ++ind) {
		                switch (ndlist[ind]) {
                        case NodeId_1: ColoredButton(ImVec4(0, 182, 208, 255), "Preprocess"); break;
                        case NodeId_2: ColoredButton(ImVec4(119, 67, 219, 255), "Parameter_Nurbs"); break;
                        case NodeId_3: ColoredButton(ImVec4(119, 67, 219, 255), "Parameter_Remesh"); break;
                        case NodeId_4: ColoredButton(ImVec4(5, 89, 91, 255), "Generator_GT"); break;
                        case NodeId_5: ColoredButton(ImVec4(5, 89, 91, 255), "Generator_ML"); break;
                        case NodeId_6: ColoredButton(ImVec4(215, 82, 129, 255), "Postprocess"); break;
		                }
                        if (ind != ndlist_sz - 1) {
                            // not the last one, exits some drawing
                            ImGui::SameLine();
                            ImGui::Text("->");
                            ImGui::SameLine();
                        }
                    }
                }, move(node_order));
            }
        }
		ImGui::PopStyleColor(3);
    }


}

NodeFlow::NodeFlow(WkflowCtxPtr wkflow_ctx): m_flow_ctx(wkflow_ctx), m_visible(true) {
    /// set reverse node_attr_id to node_id
    node_attr_records[SUBNODE(NodeId_1, 2)] = NodeId_1; // NodeId_1 only has out
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

    // preset by context
    links.clear();
    auto& ndorder = wkflow_ctx->node_order;
    auto ndorder_size = ndorder.size();
    for (auto i = 1; i < ndorder_size; ++i) {
        auto lk = LinkPair(SUBNODE(ndorder[i - 1], 2), SUBNODE(ndorder[i], 1));
        links[HASHLINK(lk)] = lk;
    }
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

void NodeFlow::delete_links(int type) {
    if (type == 1) {
        links.clear();
    }
    else {
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
}

void NodeFlow::render() {
    if (!m_visible) return;

    ImGui::Begin(("Tooth Workflow Editor - " + m_flow_ctx->flow_name).c_str());
    NodeFlowHeaders(m_flow_ctx->flow_id);

    ImNodes::BeginNodeEditor();
    // set to current context
    ImNodes::EditorContextSet(m_ctx);
    call_once(m_init, bind(&NodeFlow::reset_nodes_pos, this));

    Node_Preprocess(NodeId_1, m_flow_ctx);
    Node_Pmtr_Nurbs(NodeId_2, m_flow_ctx);
    Node_Pmtr_Remesh(NodeId_3, m_flow_ctx);
    Node_Generator_GT(NodeId_4, m_flow_ctx);
    Node_Generator_ML(NodeId_5, m_flow_ctx);
    Node_Postprocess(NodeId_6, m_flow_ctx);
    
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
        delete_links(0);
    }
    // add new link, render in next frame
    int start_attr = -1, end_attr = -1;
    if (ImNodes::IsLinkCreated(&start_attr, &end_attr)) {
        auto lp = LinkPair(start_attr, end_attr);
        links[HASHLINK(lp)] = lp;
    }
    ImGui::End();
}


void NodeFlow::get_links(vector<LinkPair>& _links) const {
    // [m_] links is storing <node_attr_id, node_attr_id>, need convert to <node_id, node_id>
    for (const auto& [_lid, _pair] : links) {
        int firstn = 0, secondn = 0;
        if (node_attr_records.find(_pair.first) != node_attr_records.end()) {
            firstn = node_attr_records.at(_pair.first);
        }
        if (node_attr_records.find(_pair.second) != node_attr_records.end()) {
            secondn = node_attr_records.at(_pair.second);
        }
        if (firstn > secondn) {
            swap(firstn, secondn); // ensure first <= second
        }
        auto nodepair = make_pair(firstn, secondn);
        
        // check if already exists
        if (find(_links.begin(), _links.end(), nodepair) == _links.end()) {
            _links.emplace_back(nodepair);
        }
    }
}

/// ------------------- NodeFlowManager ------------------

void NodeFlowManager::open_workflow(WkflowCtxPtr wkflow_ctx) {
    // check if flow_id is exists
    for (const auto& ndflow : st_nodeflows) {
        if (ndflow->get_flow_id() == wkflow_ctx->flow_id) {
            /// [TODO] there must be a error
            assert(0, "Should not open a nodeflow while it is opened");
        }
    }

    // add to next frame
    st_nodeflows.emplace_back(make_unique<NodeFlow>(wkflow_ctx));
}

bool NodeFlowManager::check_valiation(int flow_id, vector<NodeId>& node_order) {
    vector<LinkPair> links;

    for (auto& ndflow : st_nodeflows) {
        if (ndflow->get_flow_id() == flow_id) {
            ndflow->get_links(links);
            // handle ptr_param, and links
            sort(links.begin(), links.end());
            /// see tooth/toolkit.h - topological sort
            ToothSpace::topological_sort(links, node_order);
            break;
        }
    }

    if (node_order.empty()) return false;
	/// check node_order valid
	/// [TODO]
    if (node_order[0] != NodeId_1) return false;
    if (node_order.back() != NodeId_6) return false;
    auto result = false;
    for (auto& nd : node_order) {
        if (nd == NodeId_2 || nd == NodeId_3) {
            result = true;
            break;
        }
    }
    return result;
}

void NodeFlowManager::active(int flow_id) {
	// find ctx_ptr
	for (auto& ndflow : st_nodeflows) {
		if (ndflow->get_flow_id() == flow_id) {
			auto& wkflow_ctx = ndflow->get_wkflow_context();
			// check valid (topological sort)
			auto status = static_cast<int>(NodeFlowManager::check_valiation(flow_id, wkflow_ctx->node_order));
			
			SERVICE_INST->notify<void(int, int)>("/confirm_workflow", flow_id, status);
            
            if (status == 1) {
                // success, set 0 visible
                set_workflow_visible(flow_id, false);
            }
			break;
		}
	}
}

void NodeFlowManager::delete_selected_links() {
    for (auto& ndflow : st_nodeflows) {
        ndflow->delete_links(0);
    }
}

void NodeFlowManager::delete_all_links(int flow_id) {
    for (auto& ndflow : st_nodeflows) {
        if (ndflow->get_flow_id() == flow_id) {
            ndflow->delete_links(1);
        }
    }
}

void NodeFlowManager::render() {
    for (auto& ndflow : st_nodeflows) {
        ndflow->render();
    }
}

void NodeFlowManager::set_workflow_visible(int flow_id, bool visible) {
    for (auto& ndflow : st_nodeflows) {
        if (ndflow->get_flow_id() == flow_id) {
            ndflow->set_visible(visible);
        }
    }
}
