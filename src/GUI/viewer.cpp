#include "viewer.h"

#include <glad/glad.h>
#include <imnodes.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <xwindow.h>

#include "components/controller.h"
#include "components/logger.h"
#include "components/modal_confirm.h"
#include "components/node_flow.h"
#include "components/project_panel.h"
#include "components/zmo.h"


namespace GUISpace {
    using namespace std;
    using namespace RenderSpace;

    static bool st_gui_visible = true;

    void ImGuiViewer::setup() {
        // init ImNodes config
        ImNodes::CreateContext();
    }

    void ImGuiViewer::update(shared_ptr<RenderWindowWidget> win) {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (!st_gui_visible) {
            ImGui::Render();
            return;
        }

        Controller::render(win);
        Zmo::render(win);

        Logger::render();
        ModalConfirm::render();
        
        NodeFlowManager::render();
        ProjectPanel::render();

        ImGui::Render();
    }

    void ImGuiViewer::destroy() {
        ImNodes::DestroyContext();
    }

    void ImGuiViewer::change_visibility() {
        st_gui_visible = !st_gui_visible;
    }
}