#include "container.h"
#include <functional>
#include <iostream>
#include <chrono>
#include <numeric>
//#include <omp.h>
#include <mesh.h>
#include <line.hpp>
#include <intersection.h>

#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include "geom_ext/drawable.h"
#include "shader.h"
#include <imGuIZMOquat.h>


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
        //{
        //    auto raw_mesh = Mesh(
        //        vector<Point3f>{
        //            Point3f(-1.0f, 0.0f, -1.0f), Point3f(1.0f, 0.0f, -1.0f),
        //            Point3f(-1.0f, 0.0f, 1.0f), Point3f(1.0f, 0.0f, 1.0f)
        //        },
        //        vector<Vector3u>{Vector3u(0, 1, 2), Vector3u(1, 2, 3)}
        //    );
        //    auto mesh = make_shared<NewMeshDrawable>(raw_mesh, Vector3f(0.0f));
        //    mesh->_shader() = m_shaders["background"];
        //    mesh->get_ready();
        //    auto ID = add_drawable(mesh);
        //    printf("background mesh ID: %X\n", ID);
        //}
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

    bool RenderContainer::set_drawable_property(DrawableID id, const string& property, const std::any& value) {
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

    bool RenderContainer::pickcmd(
        Ray&& pick_ray,
        vector<DrawableID>& picked_ids,
        vector<Vector3f>& picked_points, vector<Vector3f>& picked_normals,
        glm::mat4& transf,
        bool multi
    ) {
        picked_ids.clear(); picked_points.clear(); picked_normals.clear();

        int cloest_ind = -1;
        float cloest_dist = numeric_limits<float>::max();

        auto inv_transf = glm::inverse(transf);
        auto n_ori = glm::vec3(inv_transf * glm::vec4(pick_ray.get_origin(), 1.0f));
		auto n_end = glm::vec3(inv_transf * glm::vec4(pick_ray.get_point(1.0f), 1.0f));
		auto n_ray = Ray(n_ori, n_end - n_ori);

        // only pick mesh
        for (auto& [draw_id, draw_ptr] : m_drawables) {
            if (!draw_ptr->_visible()) continue;
            if (draw_ptr->_type() != GeomTypeMesh) continue;

            vector<Point3f> _pnts;
            vector<Vector3f> _nmls;
            
            auto success = intersect(
                n_ray,
                *dynamic_pointer_cast<NewMeshDrawable>(draw_ptr)->_raw(),
                _pnts,
                _nmls,
                !multi
            );

            if (success) {
                auto picked_num = _pnts.size();
                auto _picked_ids = vector<DrawableID>(picked_num, draw_id);
                picked_ids.insert(picked_ids.end(), _picked_ids.begin(), _picked_ids.end());
                picked_points.insert(picked_points.end(), _pnts.begin(), _pnts.end());
                picked_normals.insert(picked_normals.end(), _nmls.begin(), _nmls.end());

                if (!multi) {
                    auto _dist = glm::length(pick_ray.get_origin() - _pnts[0]);
                    if (_dist < cloest_dist) {
                        cloest_dist = _dist;
                        cloest_ind = picked_ids.size() - 1;
                    }
                }
            }
        }

        // judge multi
        if (!multi && cloest_ind >= 0) {
            picked_ids.swap(vector<DrawableID>{picked_ids[cloest_ind]});
            picked_points.swap(vector<Vector3f>{picked_points[cloest_ind]});
            picked_normals.swap(vector<Vector3f>{picked_normals[cloest_ind]});
        }

        if (picked_ids.empty()) {
            return false;
        }

        return true;
    }
}
