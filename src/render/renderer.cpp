#include "renderer.h"

#include <iostream>

using namespace fundamental;

namespace RenderSpace {
    Renderer::Renderer(RenderEngine& engine, unsigned int _width, unsigned int _height):
        m_engine(engine),
        m_autobus(std::make_unique<fundamental::AutoBus>()) {
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
    } 

    Renderer::~Renderer() {
        glfwTerminate();
    }

    int Renderer::exec() {
        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        float vertices[] = {
            0.50f,  0.50f,  0.50f,
            0.51f,  0.51f,  0.51f,
            0.52f,  0.52f, 0.52f,
        };
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);

        glBindVertexArray(m_vao);

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);


        // render loop
        // -----------
        while (!glfwWindowShouldClose(m_window)) {
            // per-frame time logic
            // --------------------
            float currentFrame = static_cast<float>(glfwGetTime());
            m_win_widget.deltaTime = currentFrame - m_win_widget.lastFrame;
            m_win_widget.lastFrame = currentFrame;

            // input
            // -----
            m_win_widget.processInput(m_window);

            // render
            // ------
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

            // activate shader
            m_shader.use();

            // pass projection matrix to shader (note that in this case it could change every frame)
            glm::mat4 projection = glm::perspective(glm::radians(m_win_widget.fov), (float)m_win_widget.m_scr_width / (float)m_win_widget.m_scr_height, 0.1f, 100.0f);
            m_shader.setMat4("projection", projection);

            // camera/view transformation
            glm::mat4 view = glm::lookAt(m_win_widget.cameraPos, m_win_widget.cameraPos + m_win_widget.cameraFront, m_win_widget.cameraUp);
            m_shader.setMat4("view", view);

            // render elements
            glBindVertexArray(m_vao);
            glm::mat4 model = glm::mat4(1.0f);
            m_shader.setMat4("model", model);
            glPointSize(3.0f);
            glDrawArrays(GL_POINTS, 0, 3);

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

}