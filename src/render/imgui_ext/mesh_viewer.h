#pragma once

#include <memory>
#include <unordered_map>

namespace RenderSpace {
    class MeshDrawable;
    class RenderContext;
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
            RenderSpace::RenderContext* ctx,
            MeshMapType& meshes
        );
    
    private:
        static void render_mesh(
            RenderSpace::RenderContext* ctx,
            std::shared_ptr<RenderSpace::MeshDrawable> mesh,
            int mesh_id
        );
    };
}
