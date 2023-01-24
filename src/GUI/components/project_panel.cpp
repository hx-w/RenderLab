#include "project_panel.h"

#include <tooth_pack.h>
#include <geom_ext/drawable.h>
#include <imgui.h>

#include <vector>

#include "file_browser.h"
#include "../engine.h"
#include "../service.h"


using namespace std;
using namespace ToothSpace;
using namespace RenderSpace;

#define SERVICE_INST GUISpace::GUIEngine::get_instance()->get_service()

// same link glfw
#define GL_POINT 0x1B00
#define GL_LINE  0x1B01
#define GL_FILL  0x1B02

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

static const char* st_shade_modes[] = { "Point", "Grid", "Flat" };

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

const string imgui_name(const char* name, const string& tag) {
	return name + ("##" + tag);
}

void mesh_property_render(DrawablePtr msh, const string& msh_name) {
	/// P01 shade mode
	{
		int _curr = 0;
		switch (msh->_shade_mode()) {
		case GL_POINT: _curr = 0; break;
		case GL_LINE:  _curr = 1; break;
		case GL_FILL:  _curr = 2; break;
		}
		const float width = ImGui::GetWindowWidth();
		const float combo_width = width * 0.25f;
		ImGui::SetNextItemWidth(combo_width);
		ImGui::Combo(imgui_name("shade mode", msh_name).c_str(), &_curr, st_shade_modes, IM_ARRAYSIZE(st_shade_modes));
		switch (_curr) {
		case 0: msh->_shade_mode() = GL_POINT; break;
		case 1: msh->_shade_mode() = GL_LINE; break;
		case 2: msh->_shade_mode() = GL_FILL; break;
		}
	}
}

namespace GUISpace {
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
							auto msh = proj.meshes_inst.at(msh_id);
							ImGui::Checkbox(imgui_name("##", msh_name).c_str(), &msh->_visible());
							ImGui::SameLine();

							if (ImGui::TreeNode((void*)(intptr_t)msh_id, msh_name.c_str())) {

								mesh_property_render(msh, msh_name);

								ImGui::TreePop();
							}
						}

						ImGui::TreePop();
					}

					/// workflow stages
					if (proj_ctx->stage_curr == -1) {
						/// active first stage wen added to project panel
						next_workflow_stage(proj_ctx->flow_id);
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

	/// active next workflow stage
	void next_workflow_stage(int flow_id) {
		auto idx = 0;
		for (; idx < st_projects.size(); ++idx) {
			if (st_projects[idx].tpack->get_context()->flow_id == flow_id) {
				break;
			}
		}
		auto& proj = st_projects[idx];

		auto& node_order = proj.tpack->get_context()->node_order;
		auto& _curr = proj.tpack->get_context()->stage_curr;

		if (_curr == node_order.size() - 1) {
			// finish
			/// [TODO] finish workflow

			return;
		}

		auto nd = node_order[++_curr];

		// notify to xtooth
		SERVICE_INST->notify<void(int, int)>("/active_workflow_stage", flow_id, static_cast<int>(nd));
	}
}
