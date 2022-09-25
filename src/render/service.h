#ifndef RENDER_SERVICE_H
#define RENDER_SERVICE_H

#include <memory>
#include <string>
#include <array>
#include <unordered_map>
#include <functional>
#include <stack>
#include "shader.hpp"
#include "xwindow.h"
#include "mesh/elements.h"
#include "mesh/parameterization.h"
#include "../libs/coords.h"
#include "../infrastructure/communication/AutoBus.hpp"

namespace RenderSpace {
    class RenderWindowWidget;
#if defined(_WIN32)
    typedef std::unordered_map<std::string, std::thread::native_handle_type> ThreadMap;
#else
    typedef std::unordered_map<std::string, pthread_t> ThreadMap;
#endif

    class RenderService {
    public:
        RenderService();
        ~RenderService();

        void update_win(std::shared_ptr<RenderWindowWidget> win);
        std::shared_ptr<RenderWindowWidget> get_win() { return m_win_widget; }

        std::vector<Shader>& get_shaders() { return m_shaders; }

        void draw_all();

        void update();

        void imGui_render();

        void viewfit_mesh(const std::shared_ptr<Drawable> mesh);

        void notify_picking(const glm::vec3& origin, const glm::vec3& direction);
        void notify_clear_picking(); // refresh all picking ray
        void notify_window_resize(uint32_t width, uint32_t height);

        // return -1 if failed, *.obj supported
        int load_mesh(const std::string& name, const std::string& path);

        int create_mesh(const std::string& name, DrawableType type);
        void delete_mesh(int mesh_id);
        void refresh(int mesh_id);

        // parameterization
        void execute_param(int mesh_id, float& progress, ParamMethod mtd, int sample_num);

        // set pickable
        // 选定参数化4个特征点
        void set_pickable(bool pickable) { m_win_widget->set_pickable(pickable); };


    private:
        void setup();

        // 添加图元
        void add_vertex_raw(int mesh_id, std::array<Point, 3>&& coords);
        void add_edge_raw(int mesh_id, std::array<Point, 6>&& coords);
        void add_triangle_raw(int mesh_id, std::array<Point, 9>&& coords);

        int gen_id();

        void start_thread(std::string tname, std::function<void()>&& func);

    private:
        // 网格列表
        std::unordered_map<int, std::shared_ptr<MeshDrawable>> m_meshes_map;

        // background mesh
        std::shared_ptr<MeshDrawable> m_background_mesh;

        std::vector<Shader> m_shaders; // [default, background]

        // 交互
        std::shared_ptr<RenderWindowWidget> m_win_widget;

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
