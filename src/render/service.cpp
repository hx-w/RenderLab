#include "service.h"

#include <iostream>
#include <thread>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "imgui_ext/logger.h"
#include "imgui_ext/mesh_viewer.h"
#include "imgui_ext/controller.h"

#include "xwindow.h"
#include "context.h"

using namespace std;
using namespace fundamental;
using namespace imgui_ext;

namespace RenderSpace {
    RenderService::RenderService():
        m_autobus(make_unique<AutoBus>()) {
    }

    RenderService::~RenderService() {
        for (auto& [tname, thr] : m_thread_map) {
#if !defined(_WIN32)
            pthread_cancel(thr);
#endif
            m_thread_map.erase(tname);
        }
    }

    void RenderService::init_context(std::shared_ptr <RenderContext> ctx) {
        m_context = ctx;
        update_win(m_context->window());
    }

    void RenderService::register_methods() {
    }

    void RenderService::update_win(shared_ptr<RenderWindowWidget> win) {
        m_win_widget = win;
    }

    void RenderService::ray_pick(const glm::vec3& ori, const glm::vec3& dir) {
        for (auto& [id, ptr]: m_meshes_map) {
            ptr->pick_cmd(ori, dir, 1e-1f);
        }
    }

    void RenderService::notify_clear_picking() {
        auto _event = ContextHub::getInstance()->getEventTable<void()>();
        _event->notify(m_symbol + "/clear_picking");
    }

    void RenderService::notify_window_resize(uint32_t width, uint32_t height) {
    }

    void RenderService::draw_all() {
        for (auto& [id, ptr]: m_meshes_map) {
            ptr->draw();
        }
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
    }

    void RenderService::refresh(int mesh_id) {
        if (m_meshes_map.find(mesh_id) == m_meshes_map.end()) {
            return;
        }
        Logger::log("refresh mesh: " + m_meshes_map[mesh_id]->get_name() + "(" + to_string(mesh_id) + ")");
        m_meshes_map[mesh_id]->ready_to_update();
    }

    void RenderService::delete_mesh(int mesh_id) {
        if (m_meshes_map.find(mesh_id) == m_meshes_map.end()) {
            return;
        }
        std::lock_guard<std::mutex> lock(m_mutex);
        m_wait_deleted.push(mesh_id);
    }

    void RenderService::start_thread(string tname, function<void()>&& func) {
        std::thread thrd = std::thread(func);
        m_thread_map[tname] = thrd.native_handle();
        thrd.detach();
        std::cout << "[INFO] thread " << tname << " created" << std::endl;
    }

    void RenderService::viewfit_mesh(const shared_ptr<Drawable> mesh) {
        Logger::log("fit view to mesh: " + mesh->get_name());
        mesh->compute_BBOX();
        m_win_widget->viewfit_BBOX(mesh->get_BBOX());
    }

    int RenderService::load_mesh(const string& name, const string& path) {
        return false;
    }
}
