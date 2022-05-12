#include "service.h"
#include "../libs/coords.h"

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
        m_meshdraw.load_STL("./static/STL/JawScan.stl");

        m_nurbs.set_shader(m_shader);
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
                "    vec3 reflectDir = reflect(-lightDir, norm);\n"
                "    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);\n"
                "    vec3 specular = specularStrength * spec * lightColor;\n"
                "    vec3 result = (ambient + diffuse + specular) * objectColor;\n"
                "    FragColor = vec4(result, 1.0);\n"
                "}\n"
            )
        );
        // 如果逻辑线程计算太快，可能在下面方法注册前调用，会出错
        // 模块间通讯
        m_autobus->registerMethod<void(const Point&, const Point&)>(
            m_symbol + "/add_vertex",
            [this](const Point& pnt, const Point& clr) {
                m_nurbs.add_vertex_raw(Vertex(
                    glm::vec3(pnt.x(), pnt.y(), pnt.z()),
                    glm::vec3(clr.x(), clr.y(), clr.z()),
                    glm::vec3(0.0f, 0.0f, 0.0f)
                ));
            });

        m_autobus->registerMethod<void(unsigned int, unsigned int, unsigned int)>(
            m_symbol + "/add_triangle_by_idx",
            [this](unsigned int v1, unsigned int v2, unsigned int v3) {
                m_nurbs.add_triangle_by_idx(Triangle(v1, v2, v3));
            });
        
        m_autobus->registerMethod<void(const Point&, const Point&, const Point&, const Point&, const Point&, const Point&)>(
            m_symbol + "/add_triangle_raw",
            [this](const Point& p1, const Point& p2, const Point& p3, const Point& p1clr, const Point& p2clr, const Point& p3clr) {
                m_nurbs.add_triangle_raw(
                    Vertex(glm::vec3(p1.x(), p1.y(), p1.z()), glm::vec3(p1clr.x(), p1clr.y(), p1clr.z()), glm::vec3(0.0f, 0.0f, 0.0f)),
                    Vertex(glm::vec3(p2.x(), p2.y(), p2.z()), glm::vec3(p2clr.x(), p2clr.y(), p2clr.z()), glm::vec3(0.0f, 0.0f, 0.0f)),
                    Vertex(glm::vec3(p3.x(), p3.y(), p3.z()), glm::vec3(p3clr.x(), p3clr.y(), p3clr.z()), glm::vec3(0.0f, 0.0f, 0.0f))
                );
            });

        m_autobus->registerMethod<void()>(
            m_symbol + "/sync",
            [this]() {
                sync_all();
            });
    }

    void RenderService::draw_all() {
        m_nurbs.draw();
        m_meshdraw.draw();
    }

    void RenderService::update() {
        m_nurbs.sync();
    }

    void RenderService::sync_all() {
        cout << "当前线程ID: " << std::this_thread::get_id() << endl;
        m_nurbs.ready_to_update();
    }
}
