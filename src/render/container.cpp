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
        if (m_drawables.find(id) == m_drawables.end())
            return false;
		if (property == "shader") {
			m_drawables[id]->_shader() = any_cast<shared_ptr<Shader>>(value);
		}
        else if (property == "visible") {
            m_drawables[id]->_visible() = any_cast<bool>(value);
        }
		else if (property == "shade_mode") {
			m_drawables[id]->_shade_mode() = any_cast<uint32_t>(value);
		}
		else if (property == "color") {
			auto clr = any_cast<Vector3f>(value);
			auto& verts = m_drawables[id]->_vertices();
			for (auto& v : verts) {
				v.Color = clr;
			}
		}
        else if (property == "topo_shape") {
            m_drawables[id]->topo_shape = any_cast<pair<int, int>>(value);
        }
		else {
			return false;
		}

        m_drawables[id]->get_ready();
        return true;
    }

    shared_ptr<DrawableBase> RenderContainer::get_drawable_inst(DrawableID id) {
        if (m_drawables.find(id) == m_drawables.end()) return nullptr;
        return m_drawables.at(id);
    }

    /// pick points on triangles
    bool RenderContainer::pickcmd(
        Ray&& pick_ray,
        vector<DrawableID>& picked_ids,
        vector<Vector3f>& picked_points, vector<Vector3f>& picked_normals,
        const glm::mat4& transf,
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
                    auto _dist = glm::length(n_ray.get_origin() - _pnts[0]);
                    if (_dist < cloest_dist) {
                        cloest_dist = _dist;
                        cloest_ind = picked_ids.size() - 1;
                    }
                }
            }
        }

        // judge multi
        if (!multi && cloest_ind >= 0) {
            vector<DrawableID>{picked_ids[cloest_ind]}.swap(picked_ids);
            vector<Vector3f>{picked_points[cloest_ind]}.swap(picked_points);
            vector<Vector3f>{picked_normals[cloest_ind]}.swap(picked_normals);
        }

        if (picked_ids.empty()) {
            return false;
        }

        return true;
    }

    /// pick vertex indices
    bool RenderContainer::pickcmd(
        Ray&& pick_ray, DrawableID& draw_id, uint32_t& vert_id, const glm::mat4& transf, float thred
    ) {
        auto inv_transf = glm::inverse(transf);
        auto n_ori = glm::vec3(inv_transf * glm::vec4(pick_ray.get_origin(), 1.0f));
		auto n_end = glm::vec3(inv_transf * glm::vec4(pick_ray.get_point(1.0f), 1.0f));
		auto n_ray = Ray(n_ori, n_end - n_ori);

        bool picked = false;
        float cloest_dist = numeric_limits<float>::max();

        for (auto& [_id, draw_ptr] : m_drawables) {
            if (!draw_ptr->_visible()) continue;
            if (draw_ptr->_type() != GeomTypeMesh) continue;

			float min_dist = numeric_limits<float>::max();
            uint32_t min_vid = -1;

            const auto& verts = draw_ptr->_vertices();

            // stupid search
            auto vert_size = verts.size();
            for (auto vid = 0; vid < vert_size; ++vid) {
                const auto& v = verts[vid];
                auto dist = n_ray.get_distance(v.Position);
                if (dist > thred) continue;
                if (dist < min_dist &&
					// make sure v.Position is forward of n_ray
                    glm::angle(n_ray.get_direction(), v.Position - n_ori) < glm::pi<float>() / 2.f
                ) {
                    min_dist = dist;
                    min_vid = vid;
                }
            }

            // compute nearest
            if (min_vid == -1) continue;
            auto dist = glm::distance(n_ori, verts[min_vid].Position);
            if (dist < cloest_dist) {
                cloest_dist = dist;
                vert_id = min_vid;
                draw_id = _id;
                picked = true;
            }
        }
        
        return picked;
    }
}
