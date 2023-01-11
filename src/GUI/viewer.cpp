#include "viewer.h"

#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <xwindow.h>

#include "components/controller.h"
#include "components/logger.h"


namespace GUISpace {
    using namespace std;
    using namespace RenderSpace;

    void ImGuiViewer::setup() {
       // init file dialog [3rd-party]
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
        // imgui_ext::MeshViewer::render(this, m_meshes_map);
        Logger::render(win);

        // Rendering
        ImGui::Render();
    }

    void ImGuiViewer::destroy() {

    }
}