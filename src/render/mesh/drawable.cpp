#include "drawable.h"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>
#include "../glm_ext/curvature.hpp"
#include "../imgui_ext/logger.h"
#include "../libs/GLFW/glfw3.h"
#include "../libs/glad/glad.h"

using namespace std;

namespace RenderSpace {
Drawable::Drawable() {
    // 默认随机生成一个名字
    std::lock_guard<std::mutex> lk(m_mutex);
    m_name = "Drawable_" + to_string(rand());
    m_type = DrawableType::DRAWABLE_POINT;
    _ready_to_draw = false;
    _ready_to_update = false;
}

Drawable::~Drawable() {
    std::lock_guard<std::mutex> lk(m_mutex);
    _ready_to_draw = false;
    _ready_to_update = false;
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ebo);
    _reset();
}

Drawable::Drawable(const Drawable& element) {
    _deepcopy(element);
}

Drawable& Drawable::operator=(const Drawable& element) {
    _deepcopy(element);
    return *this;
}

void Drawable::set_shader(Shader& shader) {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_shader = shader;
}

void Drawable::set_type(DrawableType type) {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_type = type;
    if (type == DrawableType::DRAWABLE_POINT) {
        m_shade_mode = GL_POINT;
    } else if (type == DrawableType::DRAWABLE_LINE && m_shade_mode == GL_FILL) {
        m_shade_mode = GL_LINE;
    }
}

void Drawable::set_shade_mode(GLenum mode) {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_shade_mode = mode;
}

void Drawable::set_color_mode(ColorMode mode) {
    if (m_color_mode == mode)
        return;
    m_color_mode = mode;
    buf_colormap(mode);
    ready_to_update();
}

void Drawable::set_offset(glm::vec3 offset) {
    std::lock_guard<std::mutex> lk(m_mutex);
    m_offset = offset;
}

void Drawable::draw() {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (!_ready_to_draw) {
        return;
    }
    m_shader.use();
    glBindVertexArray(m_vao);

    // render elements
    glm::mat4 model = glm::mat4(1.0f);
    m_shader.setMat4("model", model);
    glPointSize(3.0f);

    m_shader.setVec3("offset", m_offset);

    // polygon mode
    glPolygonMode(GL_FRONT_AND_BACK, m_shade_mode);

    m_shader.setBool("randomColor", (m_color_mode == CM_Dynamic_Random));
    if (m_color_mode == CM_Dynamic_Random) {
        // get timestamp
        float ut = static_cast<float>(glfwGetTime());
        m_shader.setFloat("u_time", ut);
    }

    switch (m_type) {
        case DrawableType::DRAWABLE_POINT:
            m_shader.setBool("ignoreLight", true);
            glDrawArrays(GL_POINTS, 0, m_vertices.size());
            break;
        case DrawableType::DRAWABLE_LINE:
            m_shader.setBool("ignoreLight", true);
            glLineWidth(2.0f);
            if (m_ebo != 0 && m_edges.size() > 0) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
                glDrawElements(GL_LINES, m_edges.size() * 2, GL_UNSIGNED_INT,
                               0);
            } else {
                glDrawArrays(GL_LINES, 0, m_vertices.size());
            }
            glLineWidth(1.0f);
            break;
        case DrawableType::DRAWABLE_TRIANGLE:
            m_shader.setBool("ignoreLight", false);
            glLineWidth(1.0f);
            if (m_ebo != 0 && m_triangles.size() > 0) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
                glDrawElements(GL_TRIANGLES, m_triangles.size() * 3,
                               GL_UNSIGNED_INT, 0);
            } else {
                glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());
            }
            break;
        default:
            m_shader.setBool("ignoreLight", false);
            break;
    }

    glBindVertexArray(0);
}

void Drawable::sync() {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (!_ready_to_update)
        return;
    _ready_to_update = false;
    if (m_vao == 0) {
        _gen_vao();
    }

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex),
                 &m_vertices[0], GL_DYNAMIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    // color
    if (m_color_mode == CM_Original || m_color_mode == CM_Dynamic_Random) {
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void*)offsetof(Vertex, Color));
    } else {
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void*)offsetof(Vertex, BufColor));
    }
    glEnableVertexAttribArray(1);
    // normal
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(2);

    switch (m_type) {
        case DrawableType::DRAWABLE_LINE:
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_edges.size() * sizeof(Edge),
                         &m_edges[0], GL_DYNAMIC_DRAW);
            break;
        case DrawableType::DRAWABLE_TRIANGLE:
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                         m_triangles.size() * sizeof(Triangle), &m_triangles[0],
                         GL_DYNAMIC_DRAW);
            break;
        default:
            break;
    }
    _ready_to_draw = true;
}

void Drawable::_reset() {
    vector<Triangle>().swap(m_triangles);
    vector<Edge>().swap(m_edges);
    vector<Vertex>().swap(m_vertices);
}

void Drawable::_gen_vao() {
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);
}

void Drawable::ready_to_update() {
    std::lock_guard<std::mutex> lk(m_mutex);
    if (_ready_to_update)
        return;
    compute_BBOX();
    _ready_to_update = true;
}

void Drawable::set_visible(bool visible) {
    if (visible) {
        ready_to_update();
    } else {
        std::lock_guard<std::mutex> lk(m_mutex);
        _ready_to_draw = false;
    }
}

bool Drawable::is_visible() const {
    // std::lock_guard<std::mutex> lk(m_mutex);
    return _ready_to_draw;
}

void Drawable::compute_BBOX() {
    if (m_vertices.size() == 0)
        return;
    m_aabb = AABB(m_vertices[0].Position, m_vertices[0].Position);
    for (auto& v : m_vertices) {
        if (v.Position.x < m_aabb.first.x)
            m_aabb.first.x = v.Position.x;
        if (v.Position.y < m_aabb.first.y)
            m_aabb.first.y = v.Position.y;
        if (v.Position.z < m_aabb.first.z)
            m_aabb.first.z = v.Position.z;
        if (v.Position.x > m_aabb.second.x)
            m_aabb.second.x = v.Position.x;
        if (v.Position.y > m_aabb.second.y)
            m_aabb.second.y = v.Position.y;
        if (v.Position.z > m_aabb.second.z)
            m_aabb.second.z = v.Position.z;
    }
    m_aabb.first += m_offset;
    m_aabb.second += m_offset;
}

void Drawable::_deepcopy(const Drawable& element) {
    std::lock_guard<std::mutex> lk(m_mutex);
    _ready_to_draw = false;
    _ready_to_update = false;
    m_type = element.m_type;
    m_triangles.assign(element.m_triangles.begin(), element.m_triangles.end());
    m_edges.assign(element.m_edges.begin(), element.m_edges.end());
    m_vertices.assign(element.m_vertices.begin(), element.m_vertices.end());
    m_center = element.m_center;
    m_radius = element.m_radius;
    m_aabb = element.m_aabb;
}

void Drawable::buf_colormap(ColorMode mode) {
    if (mode == CM_Original || mode == CM_Dynamic_Random)
        return;
    if (mode == CM_Static_Random) {
        return;
    }
    // curvature color map
    if (m_type != DrawableType::DRAWABLE_TRIANGLE)
        return;

    switch (mode) {
        case CM_ColorMap_Gauss:
            compute_curvs(static_cast<int>(glm_ext::CURVATURE_GAUSSIAN));
            break;
        case CM_ColorMap_Mean:
            compute_curvs(static_cast<int>(glm_ext::CURVATURE_MEAN));
            break;
        default:
            return;
    }
    auto vec_size = m_vertices.size();
    for (auto vidx = 0; vidx < vec_size; ++vidx) {
        // float curv = (curvs[vidx] - min_curv) / (max_curv - min_curv);
        // color map: blue to red
        double r = 0.f, g = 0.f, b = 0.f;
        const double rone = 0.8;
        const double gone = 1.0;
        const double bone = 1.0;
        double x = m_curvs[vidx];
        //可以简单地理解：红色的曲率最大，蓝色的最小
        if (x < 1. / 8.) {
            r = 0;
            g = 0;
            b = bone * (0.5 + (x) / (1. / 8.) * 0.5);
        } else if (x < 3. / 8.) {
            r = 0;
            g = gone * (x - 1. / 8.) / (3. / 8. - 1. / 8.);
            b = bone;
        } else if (x < 5. / 8.) {
            r = rone * (x - 3. / 8.) / (5. / 8. - 3. / 8.);
            g = gone;
            b = (bone - (x - 3. / 8.) / (5. / 8. - 3. / 8.));
        } else if (x < 7. / 8.) {
            r = rone;
            g = (gone - (x - 5. / 8.) / (7. / 8. - 5. / 8.));
            b = 0;
        } else {
            r = (rone - (x - 7. / 8.) / (1. - 7. / 8.) * 0.5);
            g = 0;
            b = 0;
        }
        m_vertices[vidx].BufColor = glm::vec3(r, g, b);
    }

    // sort curvs
    std::sort(m_curvs.begin(), m_curvs.end());
}

void Drawable::compute_curvs(int mode) {
    // 对不同顶点，构造寻找邻接顶点的索引，且保证邻接点拓扑有序，围绕成环
    // 拓扑无序的邻接表 (set本身是有序的)
    vector<set<int>> adj_list;
    adj_list.resize(m_vertices.size());
    for (auto& tri : m_triangles) {
        for (int i = 0; i < 3; i++) {
            adj_list[tri.VertexIdx[i]].insert(tri.VertexIdx[(i + 1) % 3]);
            adj_list[tri.VertexIdx[i]].insert(tri.VertexIdx[(i + 2) % 3]);
        }
    }
    // 通过无序表构造拓扑有序邻接表
    vector<vector<int>> ord_adj_list;
    ord_adj_list.resize(m_vertices.size());
    auto vec_size = adj_list.size();
    for (int vidx = 0; vidx < vec_size; vidx++) {
        // 选中任意邻接点
        auto adj_vidx = *adj_list[vidx].begin();
        ord_adj_list[vidx].emplace_back(adj_vidx);
        // 通过临界点
        bool is_found = true;
        while (is_found) {
            // vidx 与 adj_vidx 邻接点集合的交，应该有1-2个点
            set<int> intsets;
            set_intersection(adj_list[adj_vidx].begin(),
                             adj_list[adj_vidx].end(), adj_list[vidx].begin(),
                             adj_list[vidx].end(),
                             inserter(intsets, intsets.begin()));
            is_found = false;
            for (auto& iv : intsets) {
                if (iv == adj_vidx)
                    continue;  // 不应该出现
                if (find(ord_adj_list[vidx].begin(), ord_adj_list[vidx].end(),
                         iv) == ord_adj_list[vidx].end()) {
                    // 不邻接表中，需要添加
                    ord_adj_list[vidx].emplace_back(iv);
                    adj_vidx = iv;
                    is_found = true;
                    break;
                }
            }
        }
    }
    // 对所有顶点，计算顶点的曲率
    m_curvs.clear();
    float min_curv = std::numeric_limits<float>::max();
    float max_curv = std::numeric_limits<float>::min();
    for (int vidx = 0; vidx < vec_size; ++vidx) {
        vector<glm::vec3> neighbors;
        for (auto& iv : ord_adj_list[vidx]) {
            neighbors.emplace_back(m_vertices[iv].Position);
        }

        float curv = glm_ext::compute_curvature(
            m_vertices[vidx].Position, neighbors,
            static_cast<glm_ext::CurvatureType>(mode));
        if (curv < min_curv)
            min_curv = curv;
        if (curv > max_curv)
            max_curv = curv;
        m_curvs.emplace_back(curv);
    }
    // 对所有顶点，归一化曲率，计算顶点的颜色
    cout << "min curvature: " << min_curv << "  max curvature: " << max_curv
         << endl;
    // normalize
    for (auto& curv : m_curvs) {
        curv = (curv - min_curv) / (max_curv - min_curv);
    }
}

void Drawable::sample_curvs(vector<float>& values, float sample_rate) const {
    values.clear();
    int vec_size = m_vertices.size();
    int sample_count = ceil(vec_size * sample_rate);
    if (sample_count < 2) return;
    int stride = std::max(static_cast<int>(floor(vec_size * 1. / (sample_count - 1))), 1);
    for (int vidx = 0; vidx < vec_size; vidx += stride) {
        values.emplace_back(m_curvs[vidx]);
    }
}

}  // namespace RenderSpace
