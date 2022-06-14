#include "meshbox.h"
#include "../libs/imgui/imgui.h"

using namespace imgui_ext;
using namespace RenderSpace;

void MeshBox::render(RenderService* service) {
    ImGui::Begin("MeshBox");
    ImGui::Text("MeshBox");
    ImGui::End();
}
