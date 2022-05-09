#include "renderer.h"
#include "elements.h"

#include <iostream>

using namespace std;
using namespace fundamental;

namespace RenderSpace {
    Renderer::Renderer(RenderEngine& engine, unsigned int _width, unsigned int _height):
        m_engine(engine),
        m_service(std::make_unique<RenderService>()) {
        m_win_widget.init(_width, _height);
        setup();
    }

    void Renderer::setup() {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        // glfw m_window creation
        // --------------------
        m_window = glfwCreateWindow(m_win_widget.m_scr_width, m_win_widget.m_scr_height, "XRender", NULL, NULL);
        if (m_window == NULL) {
            std::cout << "Failed to create GLFW m_window" << std::endl;
            glfwTerminate();
            return;
        }
        glfwMakeContextCurrent(m_window);
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        glfwSetWindowUserPointer(m_window, &m_win_widget);

        glfwSetCursorPosCallback(m_window, [](GLFWwindow* w, double xpos, double ypos) {
            static_cast<RenderWindowWidget*>(glfwGetWindowUserPointer(w))->mouse_callback(w, xpos, ypos);
        });
        glfwSetScrollCallback(m_window, [](GLFWwindow* w, double xoffset, double yoffset) {
            static_cast<RenderWindowWidget*>(glfwGetWindowUserPointer(w))->scroll_callback(w, xoffset, yoffset);
        });
        glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* w, int width, int height) {
            static_cast<RenderWindowWidget*>(glfwGetWindowUserPointer(w))->framebuffer_size_callback(w, width, height);
        });
        glfwSetMouseButtonCallback(m_window, [](GLFWwindow* w, int button, int action, int modes) {
            static_cast<RenderWindowWidget*>(glfwGetWindowUserPointer(w))->mouse_button_callback(w, button, action, modes);
        });
        // glad: load all OpenGL function pointers
        // ---------------------------------------
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return;
        }
    
        // configure global opengl state
        // -----------------------------
        glEnable(GL_DEPTH_TEST);

        // build and compile our shader zprogram
        // ------------------------------------
        m_shader.fromCode(
            (
                "#version 330 core\n"
                "layout (location = 0) in vec3 aPos;\n"
                "uniform mat4 model;\n"
                "uniform mat4 view;\n"
                "uniform mat4 projection;\n"
                "void main() {\n"
                "    gl_Position = projection * view * model * vec4(aPos, 1.0f);\n"
                "}\n"
            ),
            (
                "#version 330 core\n"
                "out vec4 FragColor;\n"
                "void main() {\n"
                "    FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
                "}\n"
            )
        );
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);
    } 

    Renderer::~Renderer() {
        glfwTerminate();
        m_service.reset();
    }

    int Renderer::exec() {
        // render loop
        // -----------
        while (!glfwWindowShouldClose(m_window)) {
            // clear
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

            // input
            // -----
            m_win_widget.processInput(m_window);

            update_transform();

            draw_vertex();

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(m_window);
            glfwPollEvents();
        }

        // optional: de-allocate all resources once they've outlived their purpose:
        // ------------------------------------------------------------------------
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_vbo);

        // glfw: terminate, clearing all previously allocated GLFW resources.
        // ------------------------------------------------------------------
        glfwTerminate();
        return 0;
    }

    void Renderer::draw_vertex() {
        const RenderVertices& vertices = m_service->get_vertices();
        glBindVertexArray(m_vao);

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * vertices.m_vertex_count, vertices.m_vertices, GL_DYNAMIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // render elements
        glBindVertexArray(m_vao);
        glm::mat4 model = glm::mat4(1.0f);
        m_shader.setMat4("model", model);
        glPointSize(3.0f);
        glDrawArrays(GL_POINTS, 0, vertices.m_vertex_count);
    }

    void Renderer::update_transform() {
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        m_win_widget.deltaTime = currentFrame - m_win_widget.lastFrame;
        m_win_widget.lastFrame = currentFrame;

        // activate shader
        m_shader.use();

        // pass projection matrix to shader (note that in this case it could change every frame)
        glm::mat4 projection = glm::perspective(glm::radians(m_win_widget.fov), (float)m_win_widget.m_scr_width / (float)m_win_widget.m_scr_height, 0.1f, 100.0f);
        m_shader.setMat4("projection", projection);

        // camera/view transformation
        glm::mat4 view = glm::lookAt(m_win_widget.cameraPos, m_win_widget.cameraPos + m_win_widget.cameraFront, m_win_widget.cameraUp);
        m_shader.setMat4("view", view);
    }
}