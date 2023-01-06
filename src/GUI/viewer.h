#ifndef GUI_VIEWER_H
#define GUI_VIEWER_H

#include <memory>

namespace RenderSpace {
    class RenderWindowWidget;
}

namespace GUISpace {

class IMGUIViewer {
public:
    static void update(std::shared_ptr<RenderSpace::RenderWindowWidget>);
};

}

#endif
