#ifndef GUI_VIEWER_H
#define GUI_VIEWER_H

#include <memory>

namespace RenderSpace {
    class RenderWindowWidget;
}

namespace GUISpace {

class ImGuiViewer {
public:
    static void setup();

    static void update(std::shared_ptr<RenderSpace::RenderWindowWidget>);

    static void destroy();

    /// hide or show gui
    static void change_visibility();
};

}

#endif
