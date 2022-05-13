#ifndef RENDER_SERVICE_H
#define RENDER_SERVICE_H

#include <memory>
#include <string>
#include <array>
#include "shader.hpp"
#include "./mesh/elements.h"
#include "../libs/coords.h"
#include "../infrastructure/communication/AutoBus.hpp"

namespace RenderSpace {
    class RenderService {
    public:
        RenderService();
        ~RenderService() = default;

        Shader& get_shader() { return m_shader; }

        void draw_all();

        void update();


    private:
        void setup();

        // 对外接口
        int create_mesh(const std::string& name);
        void refresh(int mesh_id);
        void add_triangle_raw(int mesh_id, std::array<Point, 9>&& coords);

    private:
        MeshDrawable m_nurbs;
        MeshDrawable m_meshdraw; // origin
        MeshDrawable m_disk; // target
        std::vector<std::shared_ptr<MeshDrawable>> m_meshes;

        Shader m_shader;

        std::string m_symbol = "render";
        std::unique_ptr<fundamental::AutoBus> m_autobus;
    };
}

#endif
