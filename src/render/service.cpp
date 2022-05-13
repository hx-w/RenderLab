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

        // 在这里预读取
        m_meshdraw.set_shader(m_shader);
        m_disk.set_shader(m_shader);
        m_meshdraw.set_type(DrawableType::DRAWABLE_TRIANGLE);
        m_disk.set_type(DrawableType::DRAWABLE_TRIANGLE);

        thread param_thread([&]() {
            m_meshdraw.load_STL("./static/STL/JawScan.stl");
            // Parameterization param(&m_meshdraw, &m_disk);
            // param.parameterize();
            // update();
        });
        param_thread.detach();
    }

    void RenderService::setup() {
        // 初始化 shader
        m_shader.fromCode(
            (
                "#version 330 core\n"
                "layout (location = 0) in vec3 aPos;\n"
                "layout (location = 1) in vec3 aColor;\n"
                "layout (location = 2) in vec3 aNorm;\n"
                "out vec3 objectColor;\n"
                "out vec3 Norm;\n"
                "out vec3 FragPos;\n"
                "uniform mat4 model;\n"
                "uniform mat4 view;\n"
                "uniform mat4 projection;\n"
                "void main() {\n"
                "    objectColor = aColor;\n"
                "    Norm = aNorm;\n"
                "    FragPos = vec3(model * vec4(aPos, 1.0));\n"
                "    gl_Position = projection * view * model * vec4(aPos, 1.0f);\n"
                "}\n"
            ),
            (
                "#version 330 core\n"
                "in vec3 objectColor;\n"
                "in vec3 Norm;\n"
                "in vec3 FragPos;\n"
                "out vec4 FragColor;\n"
                "uniform vec3 lightPos;\n"
                "uniform vec3 lightColor;\n"
                "uniform vec3 viewPos;\n"
                "void main() {\n"
                "    float ambientStrength = 0.1f;\n"
                "    float specularStrength = 0.5f;\n"
                "    vec3 ambient = ambientStrength * lightColor;\n"
                "    vec3 norm = normalize(Norm);\n"
                "    vec3 lightDir = normalize(lightPos - FragPos);\n"
                "    float diff = max(dot(norm, lightDir), 0.0);\n"
                "    vec3 diffuse = diff * lightColor;\n"
                "    vec3 viewDir = normalize(viewPos - FragPos);\n"
                "    vec3 halfwayDir = normalize(lightDir + viewDir);\n"
                "    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32);\n"
                "    vec3 specular = specularStrength * spec * lightColor;\n"
                "    vec3 result = (ambient + diffuse + specular) * objectColor;\n"
                "    FragColor = vec4(result, 1.0);\n"
                "}\n"
            )
        );
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

        // m_autobus->registerMethod<void(unsigned int, unsigned int, unsigned int)>(
        //     m_symbol + "/add_triangle_by_idx",
        //     [this](unsigned int v1, unsigned int v2, unsigned int v3) {
        //         m_nurbs.add_triangle_by_idx(Triangle(v1, v2, v3));
        //     });
        
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
    }

    void RenderService::draw_all() {
        m_meshdraw.draw();
        m_disk.draw();
        for (auto ptr: m_meshes) {
            ptr->draw();
        }
    }

    void RenderService::update() {
        m_meshdraw.sync();
        m_disk.sync();
        for (auto ptr: m_meshes) {
            ptr->sync();
        }
    }

    void RenderService::set_visible(bool visible) {
        for (auto ptr: m_meshes) {
            ptr->set_visible(visible);
        }
    }

    void RenderService::refresh(int mesh_id) {
        if (mesh_id < 0 || mesh_id >= m_meshes.size()) {
            return;
        }
        cout << "[INFO] 刷新网格: " << mesh_id << endl;
        m_meshes[mesh_id]->ready_to_update();
    }

    int RenderService::create_mesh(const string& name, DrawableType type) {
        int nsize = m_meshes.size();
        for (int i = 0; i < nsize; ++i) {
            if (m_meshes[i]->get_name() == name) {
                return i;
            }
        }
        auto new_mesh = make_shared<MeshDrawable>(name, type);
        new_mesh->set_shader(m_shader);
        m_meshes.emplace_back(new_mesh);
        return nsize;
    }

    void RenderService::add_vertex_raw(int mesh_id, array<Point, 3>&& coords) {
        if (mesh_id < 0 || mesh_id >= m_meshes.size()) {
            return;
        }
        m_meshes[mesh_id]->add_vertex_raw(Vertex(
            glm::vec3(coords[0].x(), coords[0].y(), coords[0].z()),
            glm::vec3(coords[1].x(), coords[1].y(), coords[1].z()),
            glm::vec3(coords[2].x(), coords[2].y(), coords[2].z())
        ));
    }

    void RenderService::add_triangle_raw(int mesh_id, array<Point, 9>&& coords) {
        if (mesh_id < 0 || mesh_id >= m_meshes.size()) {
            return;
        }
        m_meshes[mesh_id]->add_triangle_raw(
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
}
