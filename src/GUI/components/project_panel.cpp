#include "project_panel.h"

#include <tooth_pack.h>
#include <geom_ext/drawable.h>
#include <imgui.h>

#include <vector>

#include "file_browser.h"
#include "../engine.h"
#include "../service.h"


using namespace std;
using namespace GUISpace;
using namespace ToothSpace;
using namespace RenderSpace;

#define SERVICE_INST GUIEngine::get_instance()->get_service()

using ToothPackPtr = shared_ptr<ToothPack>;
using DrawablePtr = shared_ptr<DrawableBase>;

struct ProjectInst {
	ProjectInst(ToothPackPtr _tpack): tpack(_tpack) {
		auto& meshes_rec = tpack->get_meshes();

		// this could be dangerous, but easy to impl
		for (auto [_, msh_id] : meshes_rec) {
			meshes_inst[msh_id] = SERVICE_INST->slot_get_drawable_inst(msh_id);
		}
	}


	~ProjectInst() {
		// notify xrender / xtooth to delete
	}

	ToothPackPtr tpack; // mesh only has name to id
	map<uint32_t, DrawablePtr> meshes_inst;
};

static vector<ProjectInst> st_projects;

static bool show_import_modal = false; // import project

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
		if (proj.tpack->get_context()->flow_id == tpack_ptr->get_context()->flow_id) {
			return;
		}
	}
	st_projects.emplace_back(ProjectInst(tpack_ptr));
}


void ProjectPanel::render() {
	ImGui::Begin("Project Panel");

	if (ImGui::Button("Import Project")) {
		show_import_modal = !show_import_modal;
	}
	ImGui::SameLine();
	HelpMarker("or drag file in the window");

	ImGui::Spacing();

	if (ImGui::BeginTabBar("Confirmed_Workflows", ImGuiTabBarFlags_Reorderable)) {
		
		for (auto& proj : st_projects) {
			auto& proj_ctx = proj.tpack->get_context();
			auto& proj_meshes = proj.tpack->get_meshes();

			if (ImGui::BeginTabItem(proj_ctx->flow_name.c_str())) {
				if (ImGui::TreeNode("Meshes")) {
					/// [TODO] Some methods here
					ImGui::TextColored(ImVec4(255, 255, 100, 255), "[TODO] some methods here");
					for (auto& [msh_name, msh_id] : proj_meshes) {
						if (ImGui::TreeNode((void*)(intptr_t)msh_id, msh_name.c_str())) {

							ImGui::TreePop();
						}
					}

					ImGui::TreePop();
				}


				ImGui::EndTabItem();
			}
		}

		ImGui::EndTabBar();
	}

	ImGui::End();


	// import modal
	if (show_import_modal) {
#if 1
		static imgui_ext::file_browser_modal fileBrowser("Import");
		std::string path;
		if (fileBrowser.render(true, path)) {
			show_import_modal = false;
			/// [Notify] GUI/filepath_selected
			SERVICE_INST->notify<void(const string&)>("/filepath_selected", path);
		}
#else
		/// [TODO] implement windows pretty file dialogs
#endif
	}
}
