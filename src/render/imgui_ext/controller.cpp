#include "controller.h"
#include "browser.h"
#include "../service.h"
#include <imgui.h>
#include <iostream>

using namespace imgui_ext;
using namespace RenderSpace;
using namespace std;

void Controller::render(RenderService* service) {
    auto win = service->get_win();
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
    if (ImGui::Button("Import OBJ")) {
        show_import_modal = !show_import_modal;
    }
    ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
    ImGui::End();

    if (show_import_modal) {
        if (fileBrowser.render(true, path)) {
            show_import_modal = false;
            if (path.size() > 0 && path.substr(path.size() - 4, 4) == ".obj") {
                string name = path.substr(0, path.size() - 4);
#if defined(_WIN32)
                auto iter = name.find_last_of('\\');
#else
                auto iter = name.find_last_of('/');
#endif
                if (iter != string::npos) {
                    name = name.substr(iter + 1);
                }
                service->load_mesh(name, path);
            }
        }
    }
}