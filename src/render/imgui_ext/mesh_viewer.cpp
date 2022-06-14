#include "mesh_viewer.h"
#include "../libs/imgui/imgui.h"
#include "../mesh/elements.h"
#include "logger.h"
#include <iostream>

using namespace imgui_ext;
using namespace RenderSpace;
using namespace std;

static unordered_map<int, bool> _mesh_visibility;
static auto logger = imgui_ext::Logger::get_instance();

static void HelpMarker(const char* desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void MeshViewer::render(const MeshMapType& meshes) {
    ImGui::Begin("MeshViewer");
    ImGui::Text("All meshes created:");
    
    for (const auto& [_id, _mesh] : meshes) {
        if (_mesh_visibility.find(_id) == _mesh_visibility.end()) {
            _mesh_visibility[_id] = true;
        }
        // hide checkbox lable
        ImGui::Checkbox(("##" + _mesh->get_name()).c_str(), &_mesh_visibility[_id]);
        if (_mesh_visibility[_id] != _mesh->is_visible()) {
            _mesh->set_visible(_mesh_visibility[_id]);
        }
        ImGui::SameLine();
        // mesh widget
        render_mesh(_mesh);
    }

    ImGui::End();

    ImGui::ShowDemoWindow();
}

void MeshViewer::render_mesh(const std::shared_ptr<RenderSpace::MeshDrawable> mesh) {
    if (!ImGui::CollapsingHeader(mesh->get_name().c_str())) return;
    // Shade mode
    const char* items[] = { "GL_POINT", "GL_LINE", "GL_FILL" };
    int item_current = 0;
    switch (mesh->get_shade_mode()) {
        case GL_POINT:
            item_current = 0; break;
        case GL_LINE:
            item_current = 1; break;
        case GL_FILL:
            item_current = 2; break;
        default: break;
    }
    if (ImGui::Combo("shade mode", &item_current, items, IM_ARRAYSIZE(items))) {
        GLenum shade_mode = GL_POINT;
        switch (item_current) {
            case 0: shade_mode = GL_POINT; break;
            case 1: shade_mode = GL_LINE; break;
            case 2: shade_mode = GL_FILL; break;
            default: break;
        }
        mesh->set_shade_mode(shade_mode);
        logger->log("shade mode changed: " + mesh->get_name() + " => " + items[item_current]);
    }
    // Button::Fit
    ImGui::PushID(0);
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.25f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.25f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.25f, 0.8f, 0.8f));
    if (ImGui::Button("ViewFit")) {
        logger->log("fit view to mesh: " + mesh->get_name());
    }
    ImGui::PopStyleColor(3);
    ImGui::PopID();
    // Button::Delete
    ImGui::SameLine();
    ImGui::PushID(1);
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.f, 0.8f, 0.8f));
    if (ImGui::Button("Delete")) {
        logger->log("delete mesh: " + mesh->get_name());
    }
    ImGui::PopStyleColor(3);
    ImGui::PopID();
}
