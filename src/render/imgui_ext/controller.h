#pragma once

namespace RenderSpace {
    class RenderService;
};

namespace imgui_ext {
    class Controller {
    public:
        Controller() = default;
        ~Controller() = default;

        static void render(RenderSpace::RenderService*);
    };
}
