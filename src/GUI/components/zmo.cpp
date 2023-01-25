#include "zmo.h"

#include <xwindow.h>

#include <imgui.h>
#include <imGuIZMOquat.h>


using namespace std;
using namespace RenderSpace;

namespace GUISpace {
	void Zmo::render(shared_ptr<RenderWindowWidget> win) {
		ImGui::Begin(
			"gizmo window",
			(bool*)0,
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoBackground |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoScrollbar | 
			ImGuiWindowFlags_NoScrollWithMouse
		);
		ImGui::SetNextWindowBgAlpha(0.1);
		ImGui::gizmo3D("##gizmo", win->gizmo.getRotation(), win->cameraFront, 200, imguiGizmo::mode3Axes | imguiGizmo::cubeAtOrigin);
		ImGui::End();
	}
}
