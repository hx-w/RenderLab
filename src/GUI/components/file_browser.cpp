#include "file_browser.h"

#include <limits>
#include <imgui.h>

using namespace imgui_ext;

static void get_files_in_path(const fs::path& path, std::vector<file>& files) {
    files.clear();

    if (path.has_parent_path()) {
        files.push_back({
            "..",
            path.parent_path()
        });
    }

    for (const fs::directory_entry& dirEntry : fs::directory_iterator(path)) {
        const fs::path& dirPath = dirEntry.path();
        files.push_back({
            dirPath.filename().string(),
            dirPath
        });
    }
}

static const int clamp_size_t_to_int(const size_t data) {
    static const int max_int = std::numeric_limits<int>::max();
    return static_cast<int>(data > max_int ? max_int : data);
}

static bool vector_file_items_getter(void* data, int idx, const char** out_text) {
    const std::vector<file>* v = reinterpret_cast<std::vector<file>*>(data);
    const int elementCount = clamp_size_t_to_int(v->size());
    if (idx < 0 || idx >= elementCount) return false;
    *out_text = v->at(idx).alias.data();
    return true;
}

file_browser_modal::file_browser_modal(const char* title) :
    m_title(title),
    m_oldVisibility(false),
    m_selection(0),
    m_currentPath(fs::current_path()),
    m_currentPathIsDir(true) {
}

// Will return true if file selected.
const bool file_browser_modal::render(const bool isVisible, std::string& outPath) {
    constexpr int modal_flags =
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_AlwaysAutoResize;

    bool result = false;
    if (m_oldVisibility != isVisible) {
        m_oldVisibility  = isVisible;
        //Visiblity has changed.
        if (isVisible) {
            //Reset the path to the initial path.
            m_currentPath = fs::current_path();
            m_currentPathIsDir = true;

            //Update paths based on current path
            get_files_in_path(m_currentPath, m_filesInScope);

            //Make the modal visible.
            ImGui::OpenPopup(m_title);
        }
    }

    bool isOpen = true;
    if (ImGui::BeginPopupModal(m_title, &isOpen, modal_flags)) {
        if (ImGui::ListBox("##", &m_selection, vector_file_items_getter, &m_filesInScope, m_filesInScope.size(), 10)) {
            //Update current path to the selected list item.
            m_currentPath = m_filesInScope[m_selection].path;
            m_currentPathIsDir = fs::is_directory(m_currentPath);
            //If the selection is a directory, repopulate the list with the contents of that directory.
            if (m_currentPathIsDir) {
                get_files_in_path(m_currentPath, m_filesInScope);
            }
        }
        //Auto resize text wrap to popup width.
        ImGui::PushItemWidth(-1);
        ImGui::TextWrapped("%s", m_currentPath.string().data());
        ImGui::PopItemWidth();

        ImGui::Spacing();
        ImGui::SameLine(ImGui::GetWindowWidth() - 60);

        // Make the "Select" button look / act disabled if the current selection is a directory.
        if (m_currentPathIsDir) {
            static const ImVec4 disabledColor = { 0.3f, 0.3f, 0.3f, 1.0f };

            ImGui::PushStyleColor(ImGuiCol_Button, disabledColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, disabledColor);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, disabledColor);

            if (ImGui::Button("Select")) {
                ImGui::CloseCurrentPopup();
                outPath = m_currentPath.string();
                result = true;
                m_oldVisibility = false;
            }

            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
            ImGui::PopStyleColor();
        }
        else {
            if (ImGui::Button("Select")) {
                ImGui::CloseCurrentPopup();

                outPath = m_currentPath.string();
                result = true;
                m_oldVisibility = false;
            }
        }

        ImGui::EndPopup();
    }
    else {
        result = true;
        m_oldVisibility = false;
    }

    return result;
}
