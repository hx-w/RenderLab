#pragma once

#include <memory>
#include <unordered_map>

namespace RenderSpace {
    class MeshDrawable;
    class RenderService;
};

namespace imgui_ext {
    typedef std::unordered_map<
        int,
        std::shared_ptr<RenderSpace::MeshDrawable>
    > MeshMapType;

    class MeshViewer {
    public:
        MeshViewer() = default;
        ~MeshViewer() = default;

        static void render(
            const RenderSpace::RenderService* service,
            const MeshMapType& meshes
        );
    
    private:
        static void render_mesh(const std::shared_ptr<RenderSpace::MeshDrawable> mesh);
    };
}
