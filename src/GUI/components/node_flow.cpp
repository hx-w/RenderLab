#include <iostream>
#include <vector>
#include <functional>
#include <imgui.h>
#include <imnodes.h>

#include "node_flow.h"


#define SUBNODE(node_id, inc) ((std::hash<int>()(node_id) % 0xdeadbeef) + inc)

#define SHOWNODE(node_id, title, deft_color, active_color, ...) \
    ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32##deft_color); \
    ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32##active_color); \
    ImNodes::BeginNode(node_id); \
    ImNodes::BeginNodeTitleBar(); \
    ImGui::TextUnformatted(title); \
    ImNodes::EndNodeTitleBar(); \
    __VA_ARGS__ \
    ImNodes::EndNode(); \
    ImNodes::PopColorStyle(); \
    ImNodes::PopColorStyle();


using namespace std;
using namespace GUISpace;

static vector<pair<int, int>> st_node_links;

/**
 * Individual nodes.
 */
/// [Node] preprocess
void Node_Preprocess(int node_id) {
    SHOWNODE(node_id, "[Preprocess]",
        (0, 182, 248, 255), (0, 129, 201, 255),
        /* custom scripts */
        ImNodes::SetNodeGridSpacePos(node_id, ImVec2(35.0f, 177.0f));
        ImNodes::BeginOutputAttribute(SUBNODE(node_id, 1));
        ImGui::Text("output");
        ImNodes::EndOutputAttribute();     
    )
}

/// [Node] parameterization_nurbs
void Node_Pmtr_Nurbs(int node_id) {
    SHOWNODE(node_id, "[Parameter by Nurbs]",
        (119, 67, 219, 255), (59, 52, 134, 255),
        /* custom scripts */
        ImNodes::SetNodeGridSpacePos(node_id, ImVec2(202.0f, 278.0f));
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
        (119, 67, 219, 255), (59, 52, 134, 255),
        /* custom scripts */
        ImNodes::SetNodeGridSpacePos(node_id, ImVec2(202.0f, 57.0f));
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
        (5, 89, 91, 255), (6, 44, 48, 255),
        /* custom scripts */
        ImNodes::SetNodeGridSpacePos(node_id, ImVec2(476.0f, 72.0f));
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
        (5, 89, 91, 255), (6, 44, 48, 255),
        /* custom scripts */
        ImNodes::SetNodeGridSpacePos(node_id, ImVec2(476.0f, 255.0f));
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
        (215, 82, 129, 255), (185, 49, 96, 255),
        /* custom scripts */
        ImNodes::SetNodeGridSpacePos(node_id, ImVec2(691.0f, 177.0f));
        ImNodes::BeginInputAttribute(SUBNODE(node_id, 1));
        ImGui::Text("input");
        ImNodes::EndInputAttribute();
    )
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
    for (auto& _link : st_node_links) {
        auto _id = hash<int>()(_link.first) ^ hash<int>()(_link.second);
        ImNodes::Link(_id, _link.first, _link.second);
    }

    ImNodes::MiniMap(0.2, ImNodesMiniMapLocation_TopRight);
    ImNodes::EndNodeEditor();

    // add new link, render in next frame
    int start_attr = -1, end_attr = -1;
    if (ImNodes::IsLinkCreated(&start_attr, &end_attr)) {
        st_node_links.emplace_back(make_pair(start_attr, end_attr));
    }

    ImGui::End();
}
