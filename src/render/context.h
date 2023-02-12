/**
 * Context include [Service, Container, XWindow]
 */

#include <memory>
#include <string>
#include <any>
#include <map>

#include <communication/ContextHub.h>
#include <geom_types.h>

#include "service.h"

namespace geometry {
    class GeometryBase;
}

namespace RenderSpace {
    using DrawableID = uint32_t;
    using Props = std::map<std::string, std::any>;

    class RenderService;
    class DrawableBase;
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

        DrawableID ctx_add_drawable(
            std::shared_ptr<geometry::GeometryBase>,
            Props& /* properties */,
            int /* type */ = 2
        );

        bool ctx_remove_drawable(DrawableID);

        std::shared_ptr<DrawableBase> ctx_get_drawable(DrawableID);

        bool ctx_set_drawable_property(DrawableID, const std::string&, const std::any&);


        void ctx_change_interact_mode(int /* interact_mode */);

        /// pick from container
        void ctx_pick_points(const geometry::Vector3f&, const geometry::Vector3f&, bool);

        // hover = true, click = false
        void ctx_pick_vertex(const geometry::Vector3f&, const geometry::Vector3f&, bool);

        void ctx_update_transform_mat(const glm::mat4&);

        template <class Func, class ...Args>
        void ctx_notify(const std::string& addr, Args&&... args) {
            m_service->notify<Func>(addr, std::forward<Args>(args)...);
        }

    private:
        std::shared_ptr<RenderService> m_service;
        std::shared_ptr<RenderContainer> m_container;
        std::shared_ptr<RenderWindowWidget> m_window;
    };
}
