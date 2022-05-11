#include "service.h"
#include "../libs/coords.h"

#include <iostream>

using namespace std;
using namespace fundamental;

namespace RenderSpace {
    RenderService::RenderService():
        m_autobus(make_unique<AutoBus>()) {
        setup();

        // 在这里预读取
        m_meshdraw.set_shader(m_shader);
        m_meshdraw.load_STL("./static/STL/LowerJawScan.stl");
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
        // 模块间通讯
        m_autobus->registerMethod<void(const Point& pnt, const Point& clr)>(
            m_symbol + "/add_point",
            [this](const Point& pnt, const Point& clr) {
                m_vertices.add_vertex(pnt, clr);
            });
    }
}
