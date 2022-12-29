/**
 * Context include [Service, Container, XWindow]
 */

#include <memory>

namespace RenderSpace {

    class RenderService;
    class RenderContainer;
    class RenderWindowWidget;

    class RenderContext {
    public:
        RenderContext() = delete;

        RenderContext(std::shared_ptr<RenderService>,
                      std::shared_ptr<RenderContainer>,
                      std::shared_ptr<RenderWindowWidget>);

        ~RenderContext() = default;

        decltype(auto) service() { return m_service; }
        decltype(auto) container() { return m_container; }
        decltype(auto) window() { return m_window; }

    private:
        std::shared_ptr<RenderService> m_service;
        std::shared_ptr<RenderContainer> m_container;
        std::shared_ptr<RenderWindowWidget> m_window;
    };
}