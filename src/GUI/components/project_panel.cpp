#include "project_panel.h"

#include <tooth_pack.h>
#include <geom_ext/drawable.h>
#include <imgui.h>
#include <tinynurbs/tinynurbs.h>

#include <vector>
#include <mesh.h>

#include "file_browser.h"
#include "../engine.h"
#include "../service.h"
#include "logger.h"


using namespace std;
using namespace ToothSpace;
using namespace RenderSpace;
using namespace geometry;

#define SERVICE_INST GUISpace::GUIEngine::get_instance()->get_service()

// same link glfw
#define GL_POINT 0x1B00
#define GL_LINE  0x1B01
#define GL_FILL  0x1B02

using ToothPackPtr = shared_ptr<ToothPack>;
using DrawablePtr = shared_ptr<DrawableBase>;

static int st_shown_flow_id = -1;
const static auto st_arrow_color_default = geometry::Vector3f(0.1f, 0.1f, 0.1f); // from workspace.cpp
const static auto st_arrow_color_selected = geometry::Vector3f(1.f, 0.3f, 0.f);


const string imgui_name(const char* name, const string& tag) {
	return name + ("##" + tag);
}

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

	void render_nurbs_table() {
		auto& ctx = tpack->get_context();
		if (ctx->stage_curr < 0 ||
			ctx->stage_curr >= ctx->node_order.size() ||
			static_cast<int>(ctx->node_order[ctx->stage_curr]) < static_cast<int>(NodeId_2) ||
			static_cast<int>(ctx->node_order[ctx->stage_curr]) == static_cast<int>(NodeId_3)) {
			return;
		}
		auto max_row = any_cast<int>(ctx->node_states[NodeId_2]["Samples U"]);
		auto max_col = any_cast<int>(ctx->node_states[NodeId_2]["Samples V"]);
		if (picked_nurbs_points.empty()) {
			vector<pair<uint32_t, geometry::Point3f>>(
				max_row * max_col,
				make_pair(uint32_t(-1), geometry::Point3f(0.0f))
			).swap(picked_nurbs_points);
		}

		const static auto nurbs_table_flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::TextColored(ImVec4(0.1f, 1.f, 0.f, 1.f), "Nurbs - Picked points");
		if (ImGui::BeginTable(
			imgui_name("Sample points - Nurbs", tpack->get_context()->flow_name).c_str(),
			max_col, nurbs_table_flags
		)) {
			int counter = 0;
			for (int row = 0; row < max_row; ++row) {
				ImGui::TableNextRow();
				for (int column = 0; column < max_col; ++column) {
					ImGui::TableSetColumnIndex(column);
					auto idx = row * max_col + column;
					if (picked_nurbs_points[idx].first == uint32_t(-1)) {
						ImGui::Text("-");
					}
					else {
						auto& pnt = picked_nurbs_points[idx].second;
						char buf[48] = "";
						snprintf(buf, sizeof(buf), "%.03f, %.03f, %.03f", pnt.x, pnt.y, pnt.z);
						if (ImGui::Selectable(buf, selected_points_idx == idx, ImGuiSelectableFlags_AllowDoubleClick)) {
							if (selected_points_idx >= 0) {
								// recover old color
								SERVICE_INST->slot_set_drawable_property(
									picked_nurbs_points[selected_points_idx].first,
									"color",
									st_arrow_color_default
								);
							}
							selected_points_idx = idx;
							// hightlight arrow
							SERVICE_INST->slot_set_drawable_property(
								picked_nurbs_points[idx].first,
								"color",
								st_arrow_color_selected
							);

							if (ImGui::IsMouseDoubleClicked(0)) { 
								// double click -> copy to clipboard
								ImGui::SetClipboardText(buf);
								// notice [TODO]

							}
						}
						if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
							ImGui::BeginTooltip();
							ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
							ImGui::TextUnformatted("double click -> copy to clipboard");
							ImGui::PopTextWrapPos();
							ImGui::EndTooltip();
						}
						counter++;
					}
				}
			}
			ImGui::EndTable();

			// show compute button when points full
			if (counter == max_row * max_col && \
				ImGui::Button(imgui_name("Compute nurbs surface", ctx->flow_name).c_str())) {
				// preprocess
				vector<vector<Point3f>> pack(max_row, vector<Point3f>(max_col, Point3f(0.0f)));
				
				for (auto row = 0; row < max_row; ++row) {
					for (auto col = 0; col < max_col; ++col) {
						pack[row][col] = picked_nurbs_points[row * max_col + col].second;
					}
				}
				
				auto sample_rate = std::make_pair(
					any_cast<int>(ctx->node_states[NodeId_2]["Remesh U"]),
					any_cast<int>(ctx->node_states[NodeId_2]["Remesh V"])
				);
				SERVICE_INST->notify<void(vector<vector<Point3f>>&, const pair<int, int>&)>("/send_nurbs_points_pack", pack, sample_rate);
			}
		}

		if (selected_points_idx < 0) return;
		// handle delete event
		if (ImGui::IsKeyPressed(ImGuiKey_Backspace, false) ||
			ImGui::IsKeyPressed(ImGuiKey_Delete, false) ||
			ImGui::IsKeyPressed(ImGuiKey_X, false)
		) {
			auto _id = picked_nurbs_points[selected_points_idx].first;
			SERVICE_INST->slot_remove_drawable(_id);
			// clear records
			picked_nurbs_points[selected_points_idx].first = uint32_t(-1);
			selected_points_idx = -1;
		}
	}

	void render_generator_widget() {
		auto& ctx = tpack->get_context();
		if (ctx->stage_curr < 0 ||
			ctx->stage_curr >= ctx->node_order.size() ||
			static_cast<int>(ctx->node_order[ctx->stage_curr]) < static_cast<int>(NodeId_4)) {
			return;
		}

		// select GT / ML source
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::TextColored(ImVec4(0.1f, 1.f, 0.f, 1.f), "Select tooth depth source");

		if (tpack->get_context()->proj_t == Proj_CBCT) {
			ImGui::Text("CBCT project need 4 meshes selected\n");
			
			auto& mshes = tpack->get_meshes();

			// real-time generate char**
			const auto mshes_size = mshes.size();
			char** buf = new char* [mshes_size];
			int counter = 0;
			vector<int> current_selected(4, 0);
			for (auto& [_name, _id] : mshes) {
				buf[counter] = new char[_name.size() + 1]();
				strcpy(buf[counter], _name.c_str());

				// convert selected_generator_meshes to current_selected
				for (auto i = 0; i < selected_generator_meshes.size(); ++i) {
					if (selected_generator_meshes[i] == _id) {
						current_selected[i] = counter;
						break;
					}
				}
				counter++;
			}

			{
				const float width = ImGui::GetWindowWidth();
				const float combo_width = width * 0.5f;
				ImGui::SetNextItemWidth(combo_width);
				ImGui::Combo(imgui_name("face-1(remeshed)", ctx->flow_name).c_str(), &current_selected[0], buf, mshes_size);
				ImGui::SetNextItemWidth(combo_width);
				ImGui::Combo(imgui_name("face-2", ctx->flow_name).c_str(), &current_selected[1], buf, mshes_size);
				ImGui::SetNextItemWidth(combo_width);
				ImGui::Combo(imgui_name("face-3", ctx->flow_name).c_str(), &current_selected[2], buf, mshes_size);
				ImGui::SetNextItemWidth(combo_width);
				ImGui::Combo(imgui_name("face-4", ctx->flow_name).c_str(), &current_selected[3], buf, mshes_size);
			}

			// sync to selected_generator_meshes
			selected_generator_meshes.clear();
			for (auto pidx : current_selected) {
				int cnter = 0;
				for (auto& [_name, _id] : mshes) {
					if (pidx == cnter) {
						selected_generator_meshes.emplace_back(_id);
						break;
					}
					cnter++;
				}
			}

			for (auto i = 0; i < counter; ++i) delete[] buf[i];
			delete[] buf;
		}
		else if (tpack->get_context()->proj_t == Proj_IOS) {
			ImGui::Text("IOS project need 2 meshes selected\n");
			/// [TODO]
		}

		if (ImGui::Button(imgui_name("Generate depth", ctx->flow_name).c_str())) {
			SERVICE_INST->notify<void(const vector<uint32_t>&)>("/generate_depth", selected_generator_meshes);
		}
	}

	ToothPackPtr tpack; // mesh only has name to id
	map<uint32_t, DrawablePtr> meshes_inst;

	// nurbs picked points
	vector<pair<uint32_t, geometry::Point3f>> picked_nurbs_points;
	int selected_points_idx = -1; // in table
	

	// generator picked mesh idx
	vector<uint32_t> selected_generator_meshes;
};

static vector<ProjectInst> st_projects;
static vector<uint32_t> st_wait_deleted;

static map<uint32_t, string> st_mesh_saving_path;

static bool show_import_modal = false; // import project

static const char* st_shade_modes[] = { "Point", "Grid", "Flat" };


void switch_workflow(int& old_id, int& new_id) {
	if (old_id != -1 && old_id != new_id) {
		// change context in workflow

		/// 1. change visible (picked_nurbs_points, meshes)
		auto set_visible_patch = [](ProjectInst& rec, bool visible) {
			for (auto& _pair : rec.picked_nurbs_points) {
				SERVICE_INST->slot_set_drawable_property(_pair.first, "visible", visible);
			}
			for (auto& [_id, _inst] : rec.meshes_inst) {
				_inst->_visible() = visible; // unsafe
			}
		};

		for (auto& proj : st_projects) {
			if (proj.tpack->get_context()->flow_id == old_id) {
				set_visible_patch(proj, false);
			}
			if (proj.tpack->get_context()->flow_id == new_id) {
				set_visible_patch(proj, true);
			}
		}

	}
	old_id = new_id;
}


void delete_mesh(uint32_t msh_id) {
	auto fnd = [](map<string, uint32_t>& ctn, uint32_t v) -> string {
		for (auto& [_k, _v] : ctn) {
			if (_v == v) return _k;
		}
		return "";
	};

	for (auto& proj : st_projects) {
		if (proj.meshes_inst.find(msh_id) != proj.meshes_inst.end())
			proj.meshes_inst.erase(msh_id);
		auto& meshes = proj.tpack->get_meshes();
		auto _k = fnd(meshes, msh_id);
		if (!_k.empty()) meshes.erase(_k);
	}

	SERVICE_INST->slot_remove_drawable(msh_id);
}

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

void mesh_property_render(DrawablePtr msh, const string& msh_name, uint32_t msh_id, const string& basedir) {
	/// P01 shade mode
	auto str_msh_id = to_string(msh_id);
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
		ImGui::Combo(imgui_name("shade mode", str_msh_id).c_str(), &_curr, st_shade_modes, IM_ARRAYSIZE(st_shade_modes));
		switch (_curr) {
		case 0: msh->_shade_mode() = GL_POINT; break;
		case 1: msh->_shade_mode() = GL_LINE; break;
		case 2: msh->_shade_mode() = GL_FILL; break;
		}
	}
	/// P02 save
	{
		ImGui::Spacing();
		if (st_mesh_saving_path.find(msh_id) == st_mesh_saving_path.end()) {
			if (basedir.find("\\") != string::npos) {
				st_mesh_saving_path[msh_id] = basedir + "\\static\\" + msh_name;
			}
			else {
				st_mesh_saving_path[msh_id] = basedir + "/static/" + msh_name;
			}
		}
		ImGui::InputText(imgui_name("##save_path", str_msh_id).c_str(), st_mesh_saving_path[msh_id].data(), 64);
		ImGui::SameLine();
		if (ImGui::Button(imgui_name("save", str_msh_id).c_str())) {
			// file check
			auto& save_path = st_mesh_saving_path[msh_id];
			int status = 0;
			auto msh_ptr = dynamic_pointer_cast<NewMeshDrawable>(msh);
			geometry::Mesh::save_obj(save_path, *msh_ptr->_raw(), status);
			if (status) {
				GUISpace::Logger::log(string("file: ") + save_path + " saved success");
			}
			else {
				GUISpace::Logger::log(string("file: ") + save_path + " saved failed", GUISpace::LOG_ERROR);
			}
		}
	}
	/// P03 delete
	{
		ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0 / 7.0f, 0.6f, 0.6f));
		if (ImGui::Button(imgui_name("delete", str_msh_id).c_str())) {
			/// [TODO] carefully delete
			st_wait_deleted.emplace_back(msh_id);
		}
		ImGui::PopStyleColor();
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
		// switch to new workflow
		switch_workflow(st_shown_flow_id, tpack_ptr->get_context()->flow_id);
	}


	void ProjectPanel::render() {
		ImGui::Begin("Project Panel");

		if (ImGui::Button("Import Project")) {
			show_import_modal = !show_import_modal;
		}
		ImGui::SameLine();
		HelpMarker("or drag file in the window");

		ImGui::Spacing();

		if (ImGui::BeginTabBar(
				"Confirmed_Workflows",
				ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs
			)
		) {
			for (auto& proj : st_projects) {
				auto& proj_ctx = proj.tpack->get_context();
				auto& proj_meshes = proj.tpack->get_meshes();

				if (ImGui::BeginTabItem(proj_ctx->flow_name.c_str())) {
					switch_workflow(st_shown_flow_id, proj_ctx->flow_id);
					if (ImGui::TreeNode("Meshes")) {
						/// [TODO] Some methods here
						//ImGui::TextColored(ImVec4(255, 255, 100, 255), "[TODO] some methods here");
						for (auto& [msh_name, msh_id] : proj_meshes) {
							auto msh = proj.meshes_inst.at(msh_id);
							ImGui::Checkbox(imgui_name("##", msh_name).c_str(), &msh->_visible());
							ImGui::SameLine();

							if (ImGui::TreeNode((void*)(intptr_t)msh_id, msh_name.c_str())) {

								mesh_property_render(msh, msh_name, msh_id, proj.tpack->get_basedir());

								ImGui::TreePop();
							}
						}

						ImGui::TreePop();
					}

					/// workflow stages

					proj.render_nurbs_table();

					proj.render_generator_widget();


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

		// handle delete event
		for (auto id : st_wait_deleted) {
			delete_mesh(id);
		}
		st_wait_deleted.clear();
	}

	/// active next workflow stage
	void ProjectPanel::next_workflow_stage(int flow_id) {
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

	void ProjectPanel::add_picked_nurbs_points(uint32_t arrow_id, Vector3f& point) {
		if (st_shown_flow_id == -1) {
			SERVICE_INST->slot_remove_drawable(arrow_id); // remove
		}
		// check if full
		for (auto& proj : st_projects) {
			if (proj.tpack->get_context()->flow_id == st_shown_flow_id) {
				for (auto& id_pnt : proj.picked_nurbs_points) {
					if (id_pnt.first == uint32_t(-1)) {
						id_pnt = make_pair(arrow_id, point);
						return;
					}
				}
			}
		}

		SERVICE_INST->slot_remove_drawable(arrow_id); // fulled, remove
	}

	void ProjectPanel::register_mesh(const string& mesh_name, uint32_t mesh_id) {
		if (st_shown_flow_id == -1) return;

		for (auto& proj : st_projects) {
			if (proj.tpack->get_context()->flow_id == st_shown_flow_id) {

				proj.tpack->get_meshes()[mesh_name] = mesh_id;
				proj.meshes_inst[mesh_id] = SERVICE_INST->slot_get_drawable_inst(mesh_id);
			}
		}
	}

	uint32_t ProjectPanel::get_current_flow_id() {
		return st_shown_flow_id;
	}
}
