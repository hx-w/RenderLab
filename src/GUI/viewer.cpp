#include "viewer.h"

#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <xwindow.h>

#include "components/controller.h"
#include "components/logger.h"

#include "extern/ImFileDialog.h"

namespace GUISpace {
    using namespace std;
    using namespace RenderSpace;

    void ImGuiViewer::setup() {
       // init file dialog [3rd-party]
		ifd::FileDialog::Instance().CreateTexture = [](uint8_t* data, int w, int h, char fmt) -> void* {
			GLuint tex;
			glGenTextures(1, &tex);
			glBindTexture(GL_TEXTURE_2D, tex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, (fmt == 0) ? GL_BGRA : GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, 0);
			return (void*)tex;
		};
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