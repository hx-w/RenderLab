#pragma once

#include "base.h"

namespace GUISpace {
    class Zmo: public GUIComponentBase {
    public:
        static void render(
            std::shared_ptr<RenderSpace::RenderWindowWidget>
        );
    };
}
