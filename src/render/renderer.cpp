#include "shader.hpp"
#include "renderer.h"

#include <iostream>

namespace RenderSpace {
    Renderer::Renderer(unsigned int _width, unsigned int _height) {
        m_win.init(_width, _height);
        glfwSetWindowUserPointer(m_win.m_window, &m_win);

        auto func1 = [](GLFWwindow* w, double xpos, double ypos) {
            static_cast<RenderWindow*>(glfwGetWindowUserPointer(w))->mouse_callback(w, xpos, ypos);
        };
        glfwSetCursorPosCallback(m_win.m_window, func1);

        auto func2 = [](GLFWwindow* w, double xoffset, double yoffset) {
            static_cast<RenderWindow*>(glfwGetWindowUserPointer(w))->scroll_callback(w, xoffset, yoffset);
        };
        glfwSetScrollCallback(m_win.m_window, func2);
    }

    Renderer::~Renderer() {
        glfwTerminate();
    }

    int Renderer::exec() {
        // glfwSetFramebufferSizeCallback(m_win, func);

        // glad: load all OpenGL function pointers
        // ---------------------------------------
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return 1;
        }

        // configure global opengl state
        // -----------------------------
        glEnable(GL_DEPTH_TEST);

        // build and compile our shader zprogram
        // ------------------------------------
        Shader ourShader;
        ourShader.fromCode(
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
        // Shader ourShader("./resources/shaders/default.vs", "./resources/shaders/default.fs");

        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        float vertices[] = {
            0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f, -0.5f,
        };
        unsigned int VBO, VAO;
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);


        // load and create a texture 
        // -------------------------
        int width, height, nrChannels;

        // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
        // -------------------------------------------------------------------------------------------
        ourShader.use();

        // render loop
        // -----------
        while (!glfwWindowShouldClose(m_win.m_window)) {
            // per-frame time logic
            // --------------------
            float currentFrame = static_cast<float>(glfwGetTime());
            m_win.deltaTime = currentFrame - m_win.lastFrame;
            m_win.lastFrame = currentFrame;

            // input
            // -----
            m_win.processInput();

            // render
            // ------
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

            // activate shader
            ourShader.use();

            // pass projection matrix to shader (note that in this case it could change every frame)
            glm::mat4 projection = glm::perspective(glm::radians(m_win.fov), (float)m_win.m_scr_width / (float)m_win.m_scr_height, 0.1f, 100.0f);
            ourShader.setMat4("projection", projection);

            // camera/view transformation
            glm::mat4 view = glm::lookAt(m_win.cameraPos, m_win.cameraPos + m_win.cameraFront, m_win.cameraUp);
            ourShader.setMat4("view", view);

            // render elements
            glBindVertexArray(VAO);
            glm::mat4 model = glm::mat4(1.0f);
            ourShader.setMat4("model", model);
            glPointSize(8.0f);
            glDrawArrays(GL_POINTS, 0, 3);

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(m_win.m_window);
            glfwPollEvents();
        }

        // optional: de-allocate all resources once they've outlived their purpose:
        // ------------------------------------------------------------------------
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);

        // glfw: terminate, clearing all previously allocated GLFW resources.
        // ------------------------------------------------------------------
        glfwTerminate();
        return 0;
    }

}