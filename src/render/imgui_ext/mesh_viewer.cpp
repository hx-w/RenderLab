#include "mesh_viewer.h"
#include "../libs/imgui/imgui.h"
#include "../mesh/elements.h"

using namespace imgui_ext;
using namespace RenderSpace;
using namespace std;

static unordered_map<int, bool> _mesh_visibility;

void MeshViewer::render(const MeshMapType& meshes) {
    ImGui::Begin("MeshViewer");
    ImGui::Text("All meshes created:");
    
    for (const auto& [_id, _mesh] : meshes) {
        if (_mesh_visibility.find(_id) == _mesh_visibility.end()) {
            _mesh_visibility[_id] = true;
        }
        ImGui::Checkbox(_mesh->get_name().c_str(), &_mesh_visibility[_id]);
        if (_mesh_visibility[_id] != _mesh->is_visible()) {
            _mesh->set_visible(_mesh_visibility[_id]);
        }
    }

    ImGui::End();
}
