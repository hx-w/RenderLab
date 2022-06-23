#include "service.h"

#include <iostream>
#include <thread>

#include "xwindow.h"
#include "mesh/parameterization.h"
#include "libs/imgui/imgui.h"
#include "libs/imgui/imgui_impl_glfw.h"
#include "libs/imgui/imgui_impl_opengl3.h"
#include "imgui_ext/logger.h"
#include "imgui_ext/mesh_viewer.h"
#include "imgui_ext/controller.h"
#include "../libs/coords.h"

using namespace std;
using namespace fundamental;
static auto logger = imgui_ext::Logger::get_instance();

namespace RenderSpace {
    RenderService::RenderService():
        m_autobus(make_unique<AutoBus>()) {
        setup();
        // render background
        m_background_mesh = make_shared<MeshDrawable>("_background", DrawableType::DRAWABLE_TRIANGLE);
        m_background_mesh->set_shader(m_shaders[1]);
        m_background_mesh->add_triangle_raw(
            Vertex(glm::vec3(-1.0f, 0.0f, -1.0f), glm::vec3(0.0f), glm::vec3(0.0f)),
            Vertex(glm::vec3(1.0f, 0.0f, -1.0f), glm::vec3(0.0f), glm::vec3(0.0f)),
            Vertex(glm::vec3(-1.0f, 0.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f))
        );
        m_background_mesh->add_triangle_raw(
            Vertex(glm::vec3(1.0f, 0.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f)),
            Vertex(glm::vec3(-1.0f, 0.0f, 1.0f), glm::vec3(0.0f), glm::vec3(0.0f)),
            Vertex(glm::vec3(1.0f, 0.0f, -1.0f), glm::vec3(0.0f), glm::vec3(0.0f))
        );
        m_background_mesh->set_shade_mode(GL_FILL);
        m_background_mesh->ready_to_update();
    }

    RenderService::~RenderService() {
        for (auto& [tname, thr] : m_thread_map) {
#if !defined(_WIN32)
            pthread_cancel(thr);
#endif
            m_thread_map.erase(tname);
        }
    }

    void RenderService::setup() {
        // 初始化 shader
        string shader_dir = "./resource/shader/";
#ifdef _WIN32
        shader_dir = ".\\resource\\shader\\";
#endif
        m_shaders.emplace_back(shader_dir + "default.vs", shader_dir + "default.fs");
        m_shaders.emplace_back(shader_dir + "background.vs", shader_dir + "background.fs");

        // 如果逻辑线程计算太快，可能在下面方法注册前调用，会出错
        // 模块间通讯
        m_autobus->registerMethod<int(const string&, int)>(
            m_symbol + "/create_mesh",
            [this](const string& name, int drawable_type)->int {
                return create_mesh(name, static_cast<DrawableType>(drawable_type));
            });
        
        m_autobus->registerMethod<shared_ptr<Drawable>(const string&)>(
            m_symbol + "/load_mesh",
            [this](const string& path)->shared_ptr<Drawable> {
                auto id = load_mesh(path, path);
                return m_meshes_map[id];
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

    void RenderService::update_win(shared_ptr<RenderWindowWidget> win) {
        m_win_widget = win;
    }

    void RenderService::notify_picking(const glm::vec3& ori, const glm::vec3& dir) {
        auto _event = ContextHub::getInstance()->getEventTable<void(const Point&, const Direction&)>();
        _event->notify(m_symbol + "/picking_ray", Point(ori.x, ori.y, ori.z), Direction(dir.x, dir.y, dir.z));
    }

    void RenderService::notify_clear_picking() {
        auto _event = ContextHub::getInstance()->getEventTable<void()>();
        _event->notify(m_symbol + "/clear_picking");
    }

    void RenderService::draw_all() {
        for (auto& [id, ptr]: m_meshes_map) {
            ptr->draw();
        }
        m_background_mesh->draw();
    }

    void RenderService::update() {
        // delete stack
        std::lock_guard<std::mutex> lock(m_mutex);
        while (!m_wait_deleted.empty()) {
            int id = m_wait_deleted.top(); m_wait_deleted.pop();
            m_meshes_map.erase(id);
        }
        for (auto& [id, ptr]: m_meshes_map) {
            ptr->sync();
        }
        m_background_mesh->sync();
    }

    void RenderService::refresh(int mesh_id) {
        if (m_meshes_map.find(mesh_id) == m_meshes_map.end()) {
            return;
        }
        logger->log("refresh mesh: " + m_meshes_map[mesh_id]->get_name() + "(" + to_string(mesh_id) + ")");
        m_meshes_map[mesh_id]->ready_to_update();
    }

    void RenderService::delete_mesh(int mesh_id) {
        if (m_meshes_map.find(mesh_id) == m_meshes_map.end()) {
            return;
        }
        std::lock_guard<std::mutex> lock(m_mutex);
        m_wait_deleted.push(mesh_id);
    }

    int RenderService::create_mesh(const string& name, DrawableType type) {
        int _id = gen_id();
        auto new_mesh = make_shared<MeshDrawable>(name + "-" + to_string(_id), type);
        new_mesh->set_shader(m_shaders[0]);
        m_meshes_map[_id] = new_mesh;
        logger->log("create mesh: " + name + "(" + to_string(_id) + ")");
        return _id;
    }

    int RenderService::gen_id() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_id_gen++;
    }

    void RenderService::start_thread(string tname, function<void()>&& func) {
        std::thread thrd = std::thread(func);
        m_thread_map[tname] = thrd.native_handle();
        thrd.detach();
        std::cout << "[INFO] thread " << tname << " created" << std::endl;
    }

    void RenderService::imGui_render() {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (!m_win_widget->show_gui) {
            ImGui::Render();
            return;
        }

        imgui_ext::Controller::render(this);
        imgui_ext::MeshViewer::render(this, m_meshes_map);
		logger->render();

        // Rendering
        ImGui::Render();
    }

    void RenderService::viewfit_mesh(const shared_ptr<Drawable> mesh) {
        logger->log("fit view to mesh: " + mesh->get_name());
        mesh->compute_BBOX();
        m_win_widget->viewfit_BBOX(mesh->get_BBOX());
    }

    int RenderService::load_mesh(const string& name, const string& path) {
        if (path.substr(path.size() - 4, 4) == ".obj") {
            auto _id = create_mesh(name, DrawableType::DRAWABLE_TRIANGLE);
            if (m_meshes_map.at(_id)->load_OBJ(path)) {
                return _id;
            }
        }
        return false;
    }

    void RenderService::execute_param(int mesh_id, float& progress, ParamMethod mtd, int sample_num) {
        auto mesh_name = m_meshes_map.at(mesh_id)->get_name();
        auto _id_param = create_mesh(mesh_name + "/param", DrawableType::DRAWABLE_TRIANGLE);
        auto _id_str = create_mesh(mesh_name + "/str", DrawableType::DRAWABLE_TRIANGLE);

        start_thread(mesh_name + "/param_thread", [&, _id_param, _id_str]() {
            Parameterization pmethod(
                m_meshes_map.at(mesh_id),
                m_meshes_map.at(_id_param),
                m_meshes_map.at(_id_str)
            );
            if (!pmethod.parameterize(mtd, progress, sample_num)) {
                delete_mesh(_id_param);
                delete_mesh(_id_str);
            }
        });
    }
}
