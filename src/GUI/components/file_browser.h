#pragma once

#include <vector>
#include <string>
#include <filesystem>

// With Visual Studio compiler, filesystem is still "experimental"
namespace fs = std::filesystem;

namespace imgui_ext {
    struct file {
        std::string alias;
        fs::path path;
    };

    class file_browser_modal {
    public:
        file_browser_modal(const char* title);
        ~file_browser_modal() = default;
        const bool render(const bool isVisible, std::string& outPath);
    
    private:
        static const int modal_flags;
        const char* m_title;
        bool m_oldVisibility;
        int m_selection;
        fs::path m_currentPath;
        bool m_currentPathIsDir;
        std::vector<file> m_filesInScope;
    };
};
