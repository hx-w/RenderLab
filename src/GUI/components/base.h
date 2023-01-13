#pragma once

#include <memory>

namespace RenderSpace {
    class RenderWindowWidget;
}

namespace GUISpace {
    class GUIComponentBase {
    public:
        GUIComponentBase() = default;
        ~GUIComponentBase() = default;

        static void render();
    };
}
