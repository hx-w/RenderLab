﻿#ifndef RENDER_SERVICE_H
#define RENDER_SERVICE_H

#include <memory>
#include <string>
#include <array>
#include <unordered_map>
#include <functional>
#include "shader.hpp"
#include "xwindow.h"
#include "mesh/elements.h"
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

        Shader& get_shader() { return m_shader; }

        void draw_all();

        void update();

        void imGui_render(RenderWindowWidget*);

        void notify_picking(const glm::vec3& origin, const glm::vec3& direction);
        void notify_clear_picking(); // refresh all picking ray
        void notify_window_resize(uint32_t width, uint32_t height);

    private:
        void setup();

        // 对外接口
        int create_mesh(const std::string& name, DrawableType type);
        void refresh(int mesh_id);
        void delete_mesh(int mesh_id);

        // 添加图元
        void add_vertex_raw(int mesh_id, std::array<Point, 3>&& coords);
        void add_edge_raw(int mesh_id, std::array<Point, 6>&& coords);
        void add_triangle_raw(int mesh_id, std::array<Point, 9>&& coords);

        int gen_id();

        void start_thread(std::string tname, std::function<void()>&& func);

    private:
        // 网格列表
        std::unordered_map<int, std::shared_ptr<MeshDrawable>> m_meshes_map;

        Shader m_shader; // 带光照模型的
        Shader m_shader_text; // 文本渲染着色器

        std::string m_symbol = "render";
        std::unique_ptr<fundamental::AutoBus> m_autobus;

        int m_id_gen = 0;
        std::mutex m_mutex;

        // thread manage
        ThreadMap m_thread_map;
    };
}

#endif
