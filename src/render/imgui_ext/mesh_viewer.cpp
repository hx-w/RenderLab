#include "mesh_viewer.h"
#include "logger.h"
#include "../context.h"
#include "../service.h"
#include "../mesh/elements.h"
#include <filesystem>
#include <cstring>
#include <iostream>
#include <imgui.h>

using namespace imgui_ext;
using namespace RenderSpace;
using namespace std;

static unordered_map<int, bool> _mesh_visibility;
static unordered_map<int, bool> _mesh_alive;
static unordered_map<int, string> _mesh_savepath;
// parameterization
static unordered_map<int, float> _mesh_progress;
static unordered_map<int, int> _mesh_sample_num;

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

static inline const string IMGUI_NAME(const char* name, const string& meshname) {
    char buff[32] = "";
    snprintf(buff, sizeof(buff), "%s##%s", name, meshname.c_str());
    return string(buff);
}

void MeshViewer::render(RenderContext* ctx, MeshMapType& meshes) {
    ImGui::Begin("MeshViewer");
    ImGui::Text("All meshes created:");

    for (auto& [_id, _mesh] : meshes) {
        if (_mesh_visibility.find(_id) == _mesh_visibility.end()) {
            _mesh_visibility[_id] = true;
        }
        if (_mesh_savepath.find(_id) == _mesh_savepath.end()) {
#if defined(_WIN32)
            _mesh_savepath[_id] = ".\\" + _mesh->get_name() + ".obj";
#else
            _mesh_savepath[_id] = "./" + _mesh->get_name() + ".obj";
#endif
        }
        if (_mesh_progress.find(_id) == _mesh_progress.end()) {
            _mesh_progress[_id] = -1.0f;
        }
        if (_mesh_sample_num.find(_id) == _mesh_sample_num.end()) {
            _mesh_sample_num[_id] = 100;
        }
        _mesh_alive[_id] = true;
        // hide checkbox lable
        ImGui::Checkbox(("##" + _mesh->get_name()).c_str(), &_mesh_visibility[_id]);
        if (_mesh_visibility[_id] != _mesh->is_visible()) {
            _mesh->set_visible(_mesh_visibility[_id]);
        }
        ImGui::SameLine();
        // mesh widget
        render_mesh(ctx, _mesh, _id);
    }

    ImGui::End();
    // ImGui::ShowDemoWindow();
}

void MeshViewer::render_mesh(RenderContext* ctx, shared_ptr<RenderSpace::MeshDrawable> mesh, int mesh_id) {
    bool opened = ImGui::CollapsingHeader(mesh->get_name().c_str(), &_mesh_alive[mesh_id]);
    if (!_mesh_alive[mesh_id]) {
        // ctx->service()->delete_mesh(mesh_id);
        _mesh_alive.erase(mesh_id);
        _mesh_visibility.erase(mesh_id);
        return;
    }
    if (!opened) return;
    const string mesh_name = mesh->get_name().c_str();
    // ShadeMode
    const char* shade_str[] = { "GL_POINT", "GL_LINE", "GL_FILL" };
    int shade_current = 0;
    switch (mesh->get_shade_mode()) {
        case GL_POINT:
            shade_current = 0; break;
        case GL_LINE:
            shade_current = 1; break;
        case GL_FILL:
            shade_current = 2; break;
        default: break;
    }
    if (ImGui::SliderInt(IMGUI_NAME("polygon mode", mesh_name).c_str(), &shade_current, 0, 2, shade_str[shade_current])) {
        uint32_t shade_mode = GL_POINT;
        switch (shade_current) {
            case 0: shade_mode = GL_POINT; break;
            case 1: shade_mode = GL_LINE; break;
            case 2: shade_mode = GL_FILL; break;
            default: break;
        }
        mesh->set_shade_mode(shade_mode);
        Logger::log("polygon mode changed: " + mesh->get_name() + " => " + shade_str[shade_current]);
    }
    ImGui::SameLine(); HelpMarker("Different shading modes without changing topology.");

    // ColorMode
    const char* colortypes[] = { "original", "static random", "dynamic random", "colormap(Gauss)", "colormap(Mean)" };
    auto clr_mode = static_cast<int>(mesh->get_color_mode());
    if (ImGui::Combo(IMGUI_NAME("color mode", mesh_name).c_str(), &clr_mode, colortypes, IM_ARRAYSIZE(colortypes))) {
        mesh->set_color_mode(static_cast<ColorMode>(clr_mode));
        Logger::log("color mode changed: " + mesh->get_name() + " => " + colortypes[clr_mode]);
    }
    ImGui::SameLine(); HelpMarker("Colormap depends on the curvature of the mesh, and it will be computed in realtime by now.");
    ColorMode clrmd = static_cast<ColorMode>(clr_mode);
    if (clr_mode == CM_ColorMap_Gauss || clrmd == CM_ColorMap_Mean) {
        // 绘制图表
        static float sample_rate = 0.5f;
        vector<float> values;
        mesh->sample_curvs(values, sample_rate);
		const int len = values.size() + 1;
		float* arrv = new float[len];
        for (int i = 0; i < values.size(); ++i) arrv[i] = values[i];
        ImGui::PlotLines(IMGUI_NAME("##", mesh_name).c_str(), arrv, len);
        ImGui::SameLine();
        ImGui::PushItemWidth(ImGui::GetFontSize() * 8);
        ImGui::SliderFloat("##", &sample_rate, 0.0f, 1.0f, "rate = %.3f");
        ImGui::PopItemWidth();
		delete[] arrv;
    }

    // Offset
    glm::vec3 offset = mesh->get_offset();
    if (ImGui::DragFloat3(IMGUI_NAME("offset", mesh_name).c_str(), (float*)&offset)) {
        mesh->set_offset(offset);
    }

    // Button::Fit
    ImGui::PushID(0);
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.25f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.25f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.25f, 0.8f, 0.8f));
    if (ImGui::Button(IMGUI_NAME("view fit", mesh_name).c_str())) {
        ctx->service()->viewfit_mesh(mesh);
    }
    ImGui::PopStyleColor(3);
    ImGui::PopID();

    // Button::Save
    char savepath[128] = "";
    strcpy(savepath, _mesh_savepath[mesh_id].c_str());
    ImGui::SameLine();
    ImGui::PushID(1);
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.6f, 0.6f, 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.6f, 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.6f, 0.8f, 0.8f));
    if (ImGui::Button(IMGUI_NAME("save", mesh_name).c_str())) {
        string path = string(savepath);
        if (path.size() > 0 && path.substr(path.size() - 4, 4) == ".obj") {
            if (mesh->save_OBJ(path)) {
                Logger::log("file save to `" + path + "`");
            }
        }
        else {
            Logger::log("only OBJ format supported", LOG_ERROR);
        }
    }
    ImGui::PopStyleColor(3);
    ImGui::PopID();
    // file path
    ImGui::SameLine();
    ImGui::InputText(IMGUI_NAME("##", mesh_name).c_str(), savepath, IM_ARRAYSIZE(savepath));
    _mesh_savepath[mesh_id] = string(savepath);

    // parameteraztion
    // if (ImGui::Button(IMGUI_NAME("remesh", mesh_name).c_str())) {
    //     mesh->remesh(service);
    // }

    ImGui::Separator();
}
