#include <iostream>
#include <imgui.h>
#include "controller.h"
#include "file_browser.h"

#include "../engine.h"
#include "../service.h"

#include <xwindow.h>


using namespace GUISpace;
using namespace RenderSpace;
using namespace std;


void Controller::render(shared_ptr<RenderWindowWidget> win) {
    ImGui::Begin("Controller");
    ImGui::Text("Environment:");
    ImGui::ColorEdit3("background color", (float*)&win->bgColor);
    ImGui::ColorEdit3("light color", (float*)&win->lightColor);
    ImGui::DragFloat3("light pos", (float*)&win->lightPos);
    ImGui::Separator();
    ImGui::SliderFloat("camera speed", &win->cameraSpeed, 1.0f, 15.0f); 
    ImGui::DragFloat3("camera pos", (float*)&win->cameraPos);
    ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
    ImGui::End();
}