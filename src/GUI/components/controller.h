#pragma once

#include "base.h"

namespace GUISpace {
    class Controller: public GUIComponentBase {
    public:
        static void render(
            std::shared_ptr<RenderSpace::RenderWindowWidget>
        );
    };
}
