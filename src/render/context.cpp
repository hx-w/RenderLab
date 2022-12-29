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
        m_window->init_context(800, 600, make_shared<RenderContext>(this));
        m_service->init_context(make_shared<RenderContext>(this));
    }

}  // namespace RenderSpace
