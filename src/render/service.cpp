#include "service.h"
#include "./mesh/parameterization.h"

#include <iostream>
#include <thread>

using namespace std;
using namespace fundamental;

namespace RenderSpace {
    RenderService::RenderService():
        m_autobus(make_unique<AutoBus>()) {
        setup();

        // 文本渲染器
        m_text_service = make_unique<TextService>(m_shader_text);

        // 参数化实验
        auto _id_uns = create_mesh("uns_mesh", DrawableType::DRAWABLE_TRIANGLE);
        auto _id_param = create_mesh("param_mesh", DrawableType::DRAWABLE_TRIANGLE);
        auto _id_str = create_mesh("str_mesh", DrawableType::DRAWABLE_TRIANGLE);

        m_meshes_map.at(_id_uns)->load_OBJ("static/models/uns.obj");
        thread param_thread([&]() {
            Parameterization pmethod(
                m_meshes_map[_id_uns],
                m_meshes_map[_id_param],
                m_meshes_map[_id_str]
            );
            pmethod.parameterize(ParamMethod::Laplace);
            pmethod.resample(100);
        });
        param_thread.detach();
    }

    void RenderService::setup() {
        // 初始化 shader
        string shader_dir = "./resource/shader/";
#ifdef _WIN32
        shader_dir = ".\\resource\\shader\\";
#endif
        m_shader.fromFile(shader_dir + "default.vs", shader_dir + "default.fs");
        m_shader_text.fromFile(shader_dir + "text.vs", shader_dir + "text.fs");

        // 如果逻辑线程计算太快，可能在下面方法注册前调用，会出错
        // 模块间通讯
        m_autobus->registerMethod<int(const string&, int)>(
            m_symbol + "/create_mesh",
            [this](const string& name, int drawable_type)->int {
                return create_mesh(name, static_cast<DrawableType>(drawable_type));
            });

        m_autobus->registerMethod<void(int, array<Point, 3>&&)>(
            m_symbol + "/add_vertex_raw",
            [this](int mesh_id, array<Point, 3>&& coords) {
                this->add_vertex_raw(mesh_id, move(coords));
            });

        m_autobus->registerMethod<void(int, array<Point, 6>&&)>(
            m_symbol + "/add_edge_raw",
            [this](int mesh_id, array<Point, 6>&& coords) {
                this->add_edge_raw(mesh_id, move(coords));
            });

        // { Point1, Color1, Normal1, Point2, Color2, Normal2, Point3, Color3, Normal3 }
        m_autobus->registerMethod<void(int, array<Point, 9>&&)>(
            m_symbol + "/add_triangle_raw",
            [this](int mesh_id, array<Point, 9>&& coords) {
                this->add_triangle_raw(mesh_id, move(coords));
            });

        m_autobus->registerMethod<void(int)>(
            m_symbol + "/refresh_mesh",
            [this](int mesh_id) {
                this->refresh(mesh_id);
            });
        
        m_autobus->registerMethod<void(int)>(
            m_symbol + "/delete_mesh",
            [this](int mesh_id) {
                this->delete_mesh(mesh_id);
            });
    }

    void RenderService::notify_picking(const glm::vec3& ori, const glm::vec3& dir) {
        auto _event = ContextHub::getInstance()->getEventTable<void(const Point&, const Direction&)>();
        _event->notify(m_symbol + "/picking_ray", Point(ori.x, ori.y, ori.z), Direction(dir.x, dir.y, dir.z));
    }

    void RenderService::notify_clear_picking() {
        auto _event = ContextHub::getInstance()->getEventTable<void()>();
        _event->notify(m_symbol + "/clear_picking");
    }

    void RenderService::notify_window_resize(uint32_t width, uint32_t height) {
        // auto _event = ContextHub::getInstance()->getEventTable<void(uint32_t, uint32_t)>();
        // _event->notify(m_symbol + "/window_resize", width, height);
        // 更新文本渲染器 正交投影
        m_shader_text.use();
        glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(width), 0.0f, static_cast<GLfloat>(height));
        m_shader_text.setMat4("projection", projection);

        m_text_service->update_window_size(width, height);
    }

    void RenderService::draw_all() {
        m_text_service->draw();
        for (auto [id, ptr]: m_meshes_map) {
            ptr->draw();
        }
    }

    void RenderService::update() {
        for (auto [id, ptr]: m_meshes_map) {
            ptr->sync();
        }
    }

    void RenderService::set_visible(bool visible) {
        for (auto [id, ptr]: m_meshes_map) {
            ptr->set_visible(visible);
        }
    }

    void RenderService::set_visible(int mesh_id, bool visible) {
        if (m_meshes_map.find(mesh_id) != m_meshes_map.end()) {
            m_meshes_map[mesh_id]->set_visible(visible);
        }
    }

    void RenderService::refresh(int mesh_id) {
        if (m_meshes_map.find(mesh_id) == m_meshes_map.end()) {
            return;
        }
        cout << "[INFO] refresh Drawable: " << m_meshes_map[mesh_id]->get_name() << "(" << mesh_id << ")" << endl;
        m_meshes_map[mesh_id]->ready_to_update();
    }

    void RenderService::delete_mesh(int mesh_id) {
        if (m_meshes_map.find(mesh_id) == m_meshes_map.end()) {
            return;
        }
        cout << "[INFO] delete Drawable: " << m_meshes_map[mesh_id]->get_name() << "(" << mesh_id << ")" << endl;
        // m_meshes_map[mesh_id].reset();
        m_meshes_map.erase(mesh_id);
    }

    int RenderService::create_mesh(const string& name, DrawableType type) {
        int _id = gen_id();
        auto new_mesh = make_shared<MeshDrawable>(name, type);
        new_mesh->set_shader(m_shader);
        m_meshes_map[_id] = new_mesh;
        return _id;
    }

    void RenderService::add_vertex_raw(int mesh_id, array<Point, 3>&& coords) {
        if (m_meshes_map.find(mesh_id) == m_meshes_map.end()) {
            return;
        }
        m_meshes_map[mesh_id]->add_vertex_raw(Vertex(
            glm::vec3(coords[0].x(), coords[0].y(), coords[0].z()),
            glm::vec3(coords[1].x(), coords[1].y(), coords[1].z()),
            glm::vec3(coords[2].x(), coords[2].y(), coords[2].z())
        ));
    }

    void RenderService::add_edge_raw(int mesh_id, array<Point, 6>&& coords) {
        if (m_meshes_map.find(mesh_id) == m_meshes_map.end()) {
            return;
        }
        m_meshes_map[mesh_id]->add_edge_raw(
            Vertex(
                glm::vec3(coords[0].x(), coords[0].y(), coords[0].z()),
                glm::vec3(coords[1].x(), coords[1].y(), coords[1].z()),
                glm::vec3(coords[2].x(), coords[2].y(), coords[2].z())
            ),
            Vertex(
                glm::vec3(coords[3].x(), coords[3].y(), coords[3].z()),
                glm::vec3(coords[4].x(), coords[4].y(), coords[4].z()),
                glm::vec3(coords[5].x(), coords[5].y(), coords[5].z())
            )
        );
    }

    void RenderService::add_triangle_raw(int mesh_id, array<Point, 9>&& coords) {
        if (m_meshes_map.find(mesh_id) == m_meshes_map.end()) {
            return;
        }
        m_meshes_map[mesh_id]->add_triangle_raw(
            Vertex(
                glm::vec3(coords[0].x(), coords[0].y(), coords[0].z()),
                glm::vec3(coords[1].x(), coords[1].y(), coords[1].z()),
                glm::vec3(coords[2].x(), coords[2].y(), coords[2].z())
            ),
            Vertex(
                glm::vec3(coords[3].x(), coords[3].y(), coords[3].z()),
                glm::vec3(coords[4].x(), coords[4].y(), coords[4].z()),
                glm::vec3(coords[5].x(), coords[5].y(), coords[5].z())
            ),
            Vertex(
                glm::vec3(coords[6].x(), coords[6].y(), coords[6].z()),
                glm::vec3(coords[7].x(), coords[7].y(), coords[7].z()),
                glm::vec3(coords[8].x(), coords[8].y(), coords[8].z())
            )
        );
    }

    int RenderService::gen_id() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_id_gen++;
    }

    void RenderService::clear_text(BoxRegion region) {
        m_text_service->clear_text(region);
    }

    void RenderService::add_text(BoxRegion region, RenderLine&& line) {
        m_text_service->add_text(region, move(line));
    }

    void RenderService::update_text(BoxRegion region, int line_id, RenderLine&& line) {
        m_text_service->update_text(region, line_id, move(line));
    }

    void RenderService::delete_text(BoxRegion region, int line_id) {
        m_text_service->delete_text(region, line_id);
    }
}
