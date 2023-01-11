#include <iostream>
#include <imgui.h>
#include "controller.h"
#include "file_browser.h"

#include "../engine.h"
#include "../service.h"

#include <xwindow.h>

#if defined(_WIN32)
#include <windows.h>
#endif

using namespace GUISpace;
using namespace RenderSpace;
using namespace std;

#define SERVICE_INST GUIEngine::get_instance()->get_service()

void Controller::render(shared_ptr<RenderWindowWidget> win) {
    static bool show_import_modal = false;
    static imgui_ext::file_browser_modal fileBrowser("Import");
    std::string path;
    ImGui::Begin("Controller");
    ImGui::Text("Environment:");
    ImGui::ColorEdit3("background color", (float*)&win->bgColor);
    ImGui::ColorEdit3("light color", (float*)&win->lightColor);
    ImGui::DragFloat3("light pos", (float*)&win->lightPos);
    ImGui::Separator();
    ImGui::SliderFloat("camera speed", &win->cameraSpeed, 1.0f, 15.0f); 
    ImGui::DragFloat3("camera pos", (float*)&win->cameraPos);

    ImGui::Spacing();
    if (ImGui::Button("Select Project")) {
        show_import_modal = !show_import_modal;
    }
    ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
    ImGui::End();

    if (show_import_modal) {
#if TRUE
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