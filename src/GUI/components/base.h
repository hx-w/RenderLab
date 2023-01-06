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

        // virtual
        static void render(
            std::shared_ptr<RenderSpace::RenderWindowWidget>
        );
    };
}
