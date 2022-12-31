/**
 * Context include [Service, Container, XWindow]
 */

#include <memory>
#include <string>

namespace geometry {
    class GeometryBase;
}

namespace RenderSpace {
    using DrawableID = uint32_t;

    class RenderService;
    class RenderContainer;
    class RenderWindowWidget;

    class RenderContext {
    public:
        RenderContext() = delete;

        RenderContext(std::shared_ptr<RenderService>,
                      std::shared_ptr<RenderContainer>,
                      std::shared_ptr<RenderWindowWidget>);

        RenderContext(const RenderContext&) = delete;
        RenderContext& operator=(const RenderContext&) = delete;
        ~RenderContext() = default;

        decltype(auto) service() { return m_service; }
        decltype(auto) container() { return m_container; }
        decltype(auto) window() { return m_window; }

    public:
        /// @brief update and draw all drawable objects in container
        void ctx_update_and_draw();
        /// @brief only load **MESH** drawable object from file
        DrawableID ctx_load_drawable(const std::string& filename);

        DrawableID ctx_add_drawable(std::shared_ptr<geometry::GeometryBase>);

        bool ctx_remove_drawable(DrawableID);

    private:
        std::shared_ptr<RenderService> m_service;
        std::shared_ptr<RenderContainer> m_container;
        std::shared_ptr<RenderWindowWidget> m_window;
    };
}