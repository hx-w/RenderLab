#pragma once

#include "base.h"
#include <string>

namespace GUISpace {
    class Zmo: public GUIComponentBase {
    public:
        static void render(
            std::shared_ptr<RenderSpace::RenderWindowWidget>
        );

        static void set_mouse_tooltip(const std::string&);
    };
}
