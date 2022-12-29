// impl
#include "context.h"

#include "service.h"
#include "container.h"
#include "xwindow.h"

using namespace std;
namespace RenderSpace {

    RenderContext::RenderContext(std::shared_ptr<RenderService> service,
                                 std::shared_ptr<RenderContainer> container,
                                 std::shared_ptr<RenderWindowWidget> window)
        : m_service(service), m_container(container), m_window(window) {
        // convert this pointer to shared_ptr


        m_window->init_context(800, 600, static_cast<shared_ptr<RenderContext>>(this));
        m_service->init_context(static_cast<shared_ptr<RenderContext>>(this));
    }

}  // namespace RenderSpace
