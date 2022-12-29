// impl
#include "context.h"

#include "service.h"
#include "container.h"
#include "xwindow.h"


namespace RenderSpace {

    RenderContext::RenderContext(std::shared_ptr<RenderService> service,
                                 std::shared_ptr<RenderContainer> container,
                                 std::shared_ptr<RenderWindowWidget> window)
        : m_service(service), m_container(container), m_window(window) {}

}  // namespace RenderSpace