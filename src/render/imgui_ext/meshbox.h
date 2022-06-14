#pragma once

namespace RenderSpace {
    class RenderService;
};

namespace imgui_ext {

    class MeshBox {
    public:
        MeshBox() = default;
        ~MeshBox() = default;

        static void render(RenderSpace::RenderService* service);
    };
}
