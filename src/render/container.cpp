#include "container.h"
#include <functional>
#include <iostream>
#include <chrono>
//#include <omp.h>

#include <mesh.h>

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
        m_shaders["default"] = Shader(shader_dir + "default.vs", shader_dir + "default.fs");
        m_shaders["background"] = Shader(shader_dir + "background.vs", shader_dir + "background.fs");

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
            cout << "background mesh ID: " << ID << endl;
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
        auto now = chrono::system_clock::now();
        auto timestamp = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()).count();
        for (auto i : {13, 11, 7, 5, 3, 2}) {
            // hash timestamp and drawable
            auto hash_value = hash<uint64_t>()(timestamp) * hash<DrawableBase*>()(drawable.get()) >> i;
            if (m_drawables.find(hash_value) == m_drawables.end()) {
                m_drawables[hash_value] = drawable;
                return static_cast<DrawableID>(hash_value);
            }
        }
        return static_cast<DrawableID>(timestamp * -1);
    }
}
