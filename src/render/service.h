#ifndef RENDER_SERVICE_H
#define RENDER_SERVICE_H

#include <memory>
#include <string>
#include <array>
#include <unordered_map>
#include <functional>
#include <stack>
#include "xwindow.h"
#include "shader.h"
#include "mesh/elements.h"
#include <communication/AutoBus.hpp>

namespace RenderSpace {
    class RenderWindowWidget;
    class RenderContext;
#if defined(_WIN32)
    typedef std::unordered_map<std::string, std::thread::native_handle_type> ThreadMap;
#else
    typedef std::unordered_map<std::string, pthread_t> ThreadMap;
#endif

    class RenderService {
    public:
        RenderService();
        ~RenderService();

        void init_context(std::shared_ptr<RenderContext>);

        void register_methods();

        void update_win(std::shared_ptr<RenderWindowWidget> win);
        std::shared_ptr<RenderWindowWidget> get_win() { return m_win_widget; }

        void draw_all();

        void update();

        void viewfit_mesh(const std::shared_ptr<Drawable> mesh);

        void ray_pick(const glm::vec3& origin, const glm::vec3& direction);
        void notify_clear_picking(); // refresh all picking ray
        void notify_window_resize(uint32_t width, uint32_t height);

        // return -1 if failed, *.obj supported
        int load_mesh(const std::string& name, const std::string& path);

        void delete_mesh(int mesh_id);
        void refresh(int mesh_id);

    private:
        int gen_id();

        void start_thread(std::string tname, std::function<void()>&& func);

    private:
        // 网格列表
        std::unordered_map<int, std::shared_ptr<MeshDrawable>> m_meshes_map;

        // 交互
        std::shared_ptr<RenderWindowWidget> m_win_widget;

        // context
        std::shared_ptr<RenderContext> m_context;

        std::string m_symbol = "render";
        std::unique_ptr<fundamental::AutoBus> m_autobus;

        int m_id_gen = 0;
        std::mutex m_mutex;

        std::stack<int> m_wait_deleted;

        // thread manage
        ThreadMap m_thread_map;
    };
}

#endif
