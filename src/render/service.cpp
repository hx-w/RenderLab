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
        m_meshdraw.load_STL("./static/STL/LowerJawScan.stl");

        m_nurbs.set_shader(m_shader);
    }

    void RenderService::setup() {
        // 初始化 shader
        m_shader.fromCode(
            (
                "#version 330 core\n"
                "layout (location = 0) in vec3 aPos;\n"
                "layout (location = 1) in vec3 aColor;\n"
                "out vec3 vColor;\n"
                "uniform mat4 model;\n"
                "uniform mat4 view;\n"
                "uniform mat4 projection;\n"
                "void main() {\n"
                "    vColor = aColor;\n"
                "    gl_Position = projection * view * model * vec4(aPos, 1.0f);\n"
                "}\n"
            ),
            (
                "#version 330 core\n"
                "in vec3 vColor;\n"
                "out vec4 FragColor;\n"
                "uniform vec3 lightColor;\n"
                "void main() {\n"
                "    FragColor = vec4(vColor * lightColor, 1.0);\n"
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
                    glm::vec3(clr.x(), clr.y(), clr.z())
                ));
            });

        m_autobus->registerMethod<void(unsigned int, unsigned int, unsigned int)>(
            m_symbol + "/add_triangle_by_idx",
            [this](unsigned int v1, unsigned int v2, unsigned int v3) {
                m_nurbs.add_triangle_by_idx(Triangle(v1, v2, v3));
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
