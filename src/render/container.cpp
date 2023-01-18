#include "container.h"
#include <functional>
#include <iostream>
#include <chrono>
//#include <omp.h>
#include <mesh.h>
#include "geom_ext/drawable.h"
#include "shader.h"


using namespace std;
using namespace geometry;

namespace RenderSpace {

    RenderContainer::RenderContainer() {
        _setup();
    }

    RenderContainer::~RenderContainer() {
        // clear unordered_map
        m_drawables.clear();
    }

    void RenderContainer::_setup() {
        // setup shaders
        string shader_dir = "./resource/shader/";
#ifdef _WIN32
        shader_dir = ".\\resource\\shader\\";
#endif
        m_shaders["default"] = make_shared<Shader>(shader_dir + "default.vs", shader_dir + "default.fs");
        m_shaders["background"] = make_shared<Shader>(shader_dir + "background.vs", shader_dir + "background.fs");

        // setup background mesh
        {
            auto raw_mesh = Mesh(
                vector<Point3f>{
                    Point3f(-1.0f, 0.0f, -1.0f), Point3f(1.0f, 0.0f, -1.0f),
                    Point3f(-1.0f, 0.0f, 1.0f), Point3f(1.0f, 0.0f, 1.0f)
                },
                vector<Vector3u>{Vector3u(0, 1, 2), Vector3u(1, 2, 3)}
            );
            auto mesh = make_shared<NewMeshDrawable>(raw_mesh, Vector3f(0.0f));
            mesh->_shader() = m_shaders["background"];
            mesh->get_ready();
            auto ID = add_drawable(mesh);
            printf("background mesh ID: %X\n", ID);
        }
    }

    void RenderContainer::draw_all() {
        lock_guard<mutex> lock(m_mutex);
//#pragma omp parallel for
        for (auto& [id, drawable] : m_drawables) {
            drawable->draw();
        }
    }

    void RenderContainer::update_all() {
        lock_guard<mutex> lock(m_mutex);
        for (auto& [id, drawable] : m_drawables) {
            drawable->update();
        }
    }

    DrawableID RenderContainer::add_drawable(std::shared_ptr<DrawableBase> drawable) {
        lock_guard<mutex> lock(m_mutex);
        auto now = chrono::system_clock::now();
        auto timestamp = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();
        for (auto i : {13, 11, 7, 5, 3, 2}) {
            // hash timestamp and drawable
            auto _h = 0xdeadbeef ^ hash<uint64_t>()(timestamp);
            _h = _h ^ (hash<DrawableBase*>()(drawable.get()) >> i);
            if (m_drawables.find(_h) == m_drawables.end()) {
                m_drawables[_h] = drawable;
                return static_cast<DrawableID>(_h);
            }
        }
        return static_cast<DrawableID>(timestamp * -1);
    }

    bool RenderContainer::remove_drawable(DrawableID id) {
        lock_guard<mutex> lock(m_mutex);
        if (m_drawables.find(id) != m_drawables.end()) {
            m_drawables.erase(id);
            return true;
        }
        return false;
    }

    bool RenderContainer::set_drawable_property(DrawableID id, const string& property, const any& value) {
        lock_guard<mutex> lock(m_mutex);
        if (m_drawables.find(id) != m_drawables.end()) {
            if (property == "visible") {
                // m_drawables[id]->_visible() = any_cast<bool>(value);
                return true;
            }
            else if (property == "shader") {
                m_drawables[id]->_shader() = any_cast<shared_ptr<Shader>>(value);
                return true;
            }
            else if (property == "offset") {
                m_drawables[id]->_offset() = any_cast<Vector3f>(value);
                return true;
            }
            else if (property == "shade_mode") {
                m_drawables[id]->_shade_mode() = any_cast<uint32_t>(value);
                return true;
            }
            else {
                return false;
            }
        }
        return false;
    }

    shared_ptr<DrawableBase> RenderContainer::get_drawable_inst(DrawableID id) {
        if (m_drawables.find(id) == m_drawables.end()) return nullptr;
        return m_drawables.at(id);
    }
}
