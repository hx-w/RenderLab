#include <iostream>
#include <map>
#include <functional>
#include <imgui.h>
#include <imnodes.h>

#include "node_flow.h"


#define SUBNODE(node_id, inc) ((std::hash<int>()(node_id) % 0xdeadbeef) + inc)

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


/// mantain links
using LinkPair = pair<int, int>;
static map<int, LinkPair> st_node_links;

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

void NodeFlow::init() {
    ImNodes::SetNodeGridSpacePos(1 << 1, ImVec2(35.0f, 177.0f));
    ImNodes::SetNodeGridSpacePos(1 << 2, ImVec2(202.0f, 278.0f));
    ImNodes::SetNodeGridSpacePos(1 << 3, ImVec2(202.0f, 57.0f));
    ImNodes::SetNodeGridSpacePos(1 << 4, ImVec2(476.0f, 72.0f));
    ImNodes::SetNodeGridSpacePos(1 << 5, ImVec2(476.0f, 255.0f));
    ImNodes::SetNodeGridSpacePos(1 << 6, ImVec2(691.0f, 177.0f));
}

void NodeFlow::delete_selected_links() {
    const int num_selected_links = ImNodes::NumSelectedLinks();
    if (num_selected_links > 0) {
        std::vector<int> selected_links(num_selected_links, 0);
        ImNodes::GetSelectedLinks(selected_links.data());
        
        for (auto lid : selected_links) {
            // carefully delete
            if (st_node_links.find(lid) != st_node_links.end()) {
                st_node_links.erase(lid);
            }
        }
    }
}

void NodeFlow::render() {
    /// A testcase
    ImGui::Begin("Tooth Workflow Editor");

    ImNodes::BeginNodeEditor();

    Node_Preprocess(1 << 1);
    Node_Pmtr_Nurbs(1 << 2);
    Node_Pmtr_Remesh(1 << 3);
    Node_Generator_GT(1 << 4);
    Node_Generator_ML(1 << 5);
    Node_Postprocess(1 << 6);
    
    // create links for each frame
    for (auto& [_id, _link] : st_node_links) {
        ImNodes::Link(_id, _link.first, _link.second);
    }

    ImNodes::MiniMap(0.2, ImNodesMiniMapLocation_TopRight);
    ImNodes::EndNodeEditor();

    // add new link, render in next frame
    int start_attr = -1, end_attr = -1;
    if (ImNodes::IsLinkCreated(&start_attr, &end_attr)) {
        auto lp = LinkPair(start_attr, end_attr);
        st_node_links[HASHLINK(lp)] = lp;
    }

    ImGui::End();
}
