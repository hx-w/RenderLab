#include <iostream>
#include <vector>
#include <functional>
#include <imgui.h>
#include <imnodes.h>

#include "node_flow.h"


using namespace std;
using namespace GUISpace;

static vector<pair<int, int>> st_node_links;

void NodeFlow::render() {
    /// A testcase
    ImGui::Begin("simple node editor");

    ImNodes::BeginNodeEditor();

    {
        ImNodes::BeginNode(1);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("simple node :)");
        ImNodes::EndNodeTitleBar();

        ImNodes::BeginInputAttribute(10);
        ImGui::Text("input");
        ImNodes::EndInputAttribute();

        ImNodes::BeginOutputAttribute(11);
        ImGui::Indent(40);
        ImGui::Text("output");
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();
    }
    {
        ImNodes::BeginNode(2);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("the 2nd one");
        ImNodes::EndNodeTitleBar();

        ImNodes::BeginInputAttribute(20);
        ImGui::Text("input");
        ImNodes::EndInputAttribute();

        ImNodes::BeginOutputAttribute(21);
        ImGui::Indent(40);
        ImGui::Text("output");
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();
    }

    
    // create links for each frame
    for (auto& _link : st_node_links) {
        auto _id = hash<int>()(_link.first) ^ hash<int>()(_link.second);
        ImNodes::Link(_id, _link.first, _link.second);
    }

    ImNodes::EndNodeEditor();


    // add new link, render in next frame
    int start_attr = -1, end_attr = -1;
    if (ImNodes::IsLinkCreated(&start_attr, &end_attr)) {
        st_node_links.emplace_back(make_pair(start_attr, end_attr));
    }

    ImGui::End();
}
