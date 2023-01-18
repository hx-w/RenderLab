#include "project_panel.h"

#include <tooth_pack.h>
#include <imgui.h>

#include <vector>

using namespace std;
using namespace GUISpace;
using namespace ToothSpace;


static vector<shared_ptr<ToothPack>> st_projects;

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

void ProjectPanel::add_tooth_pack(shared_ptr<ToothPack> tpack_ptr) {
	// carefull add
	for (auto& proj : st_projects) {
		if (proj->get_context()->flow_id == tpack_ptr->get_context()->flow_id) {
			return;
		}
	}
	st_projects.emplace_back(tpack_ptr);
}


void ProjectPanel::render() {
	ImGui::Begin("Project Panel");

	ImGui::Button("Import Project");
	ImGui::SameLine();
	HelpMarker("or drag file in the window");

	if (ImGui::BeginTabBar("Confirmed_Workflows", ImGuiTabBarFlags_Reorderable)) {
		
		for (auto& proj : st_projects) {
			auto& proj_ctx = proj->get_context();
			auto& proj_meshes = proj->get_meshes();

			if (ImGui::BeginTabItem(proj_ctx->flow_name.c_str())) {
				if (ImGui::TreeNode("Meshes")) {
					

					ImGui::TreePop();
				}


				ImGui::EndTabItem();
			}
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
}
