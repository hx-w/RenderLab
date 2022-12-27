#include "container.h"
#include <functional>
#include <iostream>

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
    }

    void RenderContainer::draw_all() {
        // mutex
        lock_guard<mutex> lock(m_mutex);
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
        // hash drawable
        DrawableID id = hash<DrawableBase*>()(drawable.get());
        m_drawables[id] = drawable;
        return id;
    }
}
