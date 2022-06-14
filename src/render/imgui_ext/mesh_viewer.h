#pragma once

namespace RenderSpace {
    class RenderService;
};

namespace imgui_ext {

    class MeshViewer {
    public:
        MeshViewer() = default;
        ~MeshViewer() = default;

        static void render(RenderSpace::RenderService* service);
    };
}
