#include "mesh_viewer.h"
#include "../libs/imgui/imgui.h"

using namespace imgui_ext;
using namespace RenderSpace;

void MeshViewer::render(RenderService* service) {
    ImGui::Begin("MeshViewer");
    ImGui::Text("MeshViewer");
    ImGui::End();
}
