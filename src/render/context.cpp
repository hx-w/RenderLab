// impl
#include "context.h"

#include "service.h"
#include "container.h"
#include "xwindow.h"
#include "imgui_ext/logger.h"
#include "geom_ext/drawable.h"

#include <mesh.h>

using namespace std;
using namespace geometry;
using namespace imgui_ext;

namespace RenderSpace {

    RenderContext::RenderContext(std::shared_ptr<RenderService> service,
                                 std::shared_ptr<RenderContainer> container,
                                 std::shared_ptr<RenderWindowWidget> window)
        : m_service(service), m_container(container), m_window(window) {
        // convert this pointer to shared_ptr

        m_window->init_context(static_cast<shared_ptr<RenderContext>>(this));
        m_service->init_context(static_cast<shared_ptr<RenderContext>>(this));
    }

    void RenderContext::ctx_update_and_draw() {
        m_container->update_all();
        m_container->draw_all();
    }

    DrawableID RenderContext::ctx_load_drawable(const string& filename) {
        // only mesh
        bool status = false;
        auto geom_mesh = Mesh::load_obj(filename, status);
        if (!status) {
            Logger::log("Error: cannot load mesh from file " + filename, LOG_ERROR);
            return -1;
        }
        auto drawable_mesh = make_shared<NewMeshDrawable>(geom_mesh, Vector3f(0.2));
        auto drawable_id = m_container->add_drawable(drawable_mesh);
        Logger::log("Load mesh from file " + filename + " success", LOG_INFO);
        return drawable_id;
    }

    DrawableID RenderContext::ctx_add_drawable(shared_ptr<GeometryBase> geom) {
        // if is mesh
        auto geom_mesh = dynamic_pointer_cast<Mesh>(geom);
        auto drawable_mesh = make_shared<NewMeshDrawable>(*geom_mesh, Vector3f(0.2));
        auto drawable_id = m_container->add_drawable(drawable_mesh);
        return drawable_id;
    }

    bool RenderContext::ctx_remove_drawable(DrawableID id) {
        return m_container->remove_drawable(id);
    }

}  // namespace RenderSpace
