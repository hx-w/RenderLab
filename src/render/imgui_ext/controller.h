#pragma once

namespace RenderSpace {
    class RenderContext;
};

namespace imgui_ext {
    class Controller {
    public:
        Controller() = default;
        ~Controller() = default;

        static void render(RenderSpace::RenderContext*);
    };
}
