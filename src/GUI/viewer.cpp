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


namespace GUISpace {
    using namespace std;
    using namespace RenderSpace;

    void ImGuiViewer::setup() {
        // init [3rd-party]
        ImNodes::CreateContext();
        ImNodes::StyleColorsDark();
    }

    void ImGuiViewer::update(shared_ptr<RenderWindowWidget> win) {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (!win->show_gui) {
            ImGui::Render();
            return;
        }

        Controller::render(win);

        Logger::render();
        ModalConfirm::render();
        
        NodeFlow::render();

        ImGui::Render();
    }

    void ImGuiViewer::destroy() {
        ImNodes::DestroyContext();
    }
}