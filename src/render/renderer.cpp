#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "./mesh/elements.h"

#include "renderer.h"
#include "invoker.h"

#include "context.h"
#include "service.h"
#include "container.h"
#include "xwindow.h"

using namespace std;
using namespace fundamental;

namespace RenderSpace {
    Renderer::Renderer(RenderEngine& engine, unsigned int _width, unsigned int _height):
        m_engine(engine) {
        // DO NOT CHANGE THE ORDER OF THE FOLLOWING LINES
//        m_context = make_shared<RenderContext>(
//            make_shared<RenderService>(),
//            make_shared<RenderContainer>(),
//            make_shared<RenderWindowWidget>()
//        );
        setup(_width, _height);

        m_context = make_shared<RenderContext>(
                make_shared<RenderService>(),
                make_shared<RenderContainer>(),
                make_shared<RenderWindowWidget>()
        );

//        m_context->window()->init(_width, _height, m_context->service());
//        m_context->service()->update_win(m_context->window());

        // m_win_widget = std::make_shared<RenderWindowWidget>();
        // setup(_width, _height);
        // // service 中维护了 shader，需要在renderer setup之后初始化
        // m_service = std::make_shared<RenderService>();
        // m_win_widget->init(_width, _height, m_service);
        // m_service->update_win(m_win_widget);

        // m_container = std::make_shared<RenderContainer>();
    }

    void Renderer::setup(unsigned int w, unsigned int h) {
        glfwInit();
        glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

        // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
        // GL ES 2.0 + GLSL 100
        const char* glsl_version = "#version 100";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
        // GL 3.2 + GLSL 150
        const char* glsl_version = "#version 150";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
        // GL 3.0 + GLSL 130
        const char* glsl_version = "#version 130";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
        //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

        // glfw m_window creation
        // --------------------
        m_window = glfwCreateWindow(w, h, "Render Lab", NULL, NULL);
        if (m_window == nullptr) {
            std::cout << "Failed to create GLFW m_window" << std::endl;
            glfwTerminate();
            return;
        }
        glfwMakeContextCurrent(m_window);
        glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

        glfwSetWindowUserPointer(m_window, m_context->window().get());

        glfwSetCursorPosCallback(m_window, [](GLFWwindow* w, double xpos, double ypos) {
            if (!ImGui::GetIO().WantCaptureMouse)
            static_cast<RenderWindowWidget*>(glfwGetWindowUserPointer(w))->mouse_callback(w, xpos, ypos);
        });
        glfwSetScrollCallback(m_window, [](GLFWwindow* w, double xoffset, double yoffset) {
            if (!ImGui::GetIO().WantCaptureMouse)
            static_cast<RenderWindowWidget*>(glfwGetWindowUserPointer(w))->scroll_callback(w, xoffset, yoffset);
        });
        glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* w, int width, int height) {
            if (!ImGui::GetIO().WantCaptureMouse)
            static_cast<RenderWindowWidget*>(glfwGetWindowUserPointer(w))->framebuffer_size_callback(w, width, height);
        });
        glfwSetMouseButtonCallback(m_window, [](GLFWwindow* w, int button, int action, int modes) {
            if (!ImGui::GetIO().WantCaptureMouse)
            static_cast<RenderWindowWidget*>(glfwGetWindowUserPointer(w))->mouse_button_callback(w, button, action, modes);
        });
        // glad: load all OpenGL function pointers
        // ---------------------------------------
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return;
        }
    
        glEnable(GL_DEPTH_TEST);
        // 启用阴影平滑
        glShadeModel(GL_SMOOTH);
        // glEnable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glfwSwapInterval(1); // Enable vsync

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.Fonts->AddFontFromFileTTF("resource/fonts/CascadiaCodePL.ttf", 14.0f);

        // Setup Dear ImGui style
        // ImGui::StyleColorsDark();
        ImGui::StyleColorsClassic();
        // ImGui::StyleColorsLight();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);
    } 

    Renderer::~Renderer() {
        glfwTerminate();
        m_context.reset();
    }

    int Renderer::exec() {
        auto cmd_queue = CommandQueue::get_instance();

        while (!glfwWindowShouldClose(m_window)) {
            glfwPollEvents();

            cmd_queue->invoke();            

            const auto& clr = m_context->window()->bgColor;
            glClearColor(clr.x * clr.w, clr.y * clr.w, clr.z * clr.w, clr.w);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

            // input
            if (!ImGui::GetIO().WantCaptureKeyboard) {
                m_context->window()->processInput(m_window);
            }

            update_transform();

            draw();

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            glfwSwapBuffers(m_window);
        }

        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(m_window);
        glfwTerminate();
        return 0;
    }

    void Renderer::draw() {
        m_context->service()->imGui_render();

        m_context->container()->update_all();
        m_context->container()->draw_all();
    }

    void Renderer::update_transform() {
        auto& shaders = m_context->container()->shaders();
        // auto& shaders = m_service->get_shaders();
        // --------------------
        auto m_win_widget = m_context->window();
        auto currentFrame = static_cast<float>(glfwGetTime());
        m_win_widget->deltaTime = currentFrame - m_win_widget->lastFrame;
        m_win_widget->lastFrame = currentFrame;

        // activate shader
        glm::mat4 projection = glm::perspective(
            glm::radians(m_win_widget->fov),
            (float)m_win_widget->m_scr_width / (float)m_win_widget->m_scr_height,
            0.1f, 500.0f
        );
        glm::mat4 view = glm::lookAt(
            m_win_widget->cameraPos,
            m_win_widget->cameraPos + m_win_widget->cameraFront,
            m_win_widget->cameraUp
        );
        for (auto& [_, shader] : shaders) {
            shader.use();

            // pass projection matrix to shader (note that in this case it could change every frame)
            shader.setMat4("projection", projection);

            // camera/view transformation
            shader.setMat4("view", view);

            shader.setVec3("lightColor",  m_win_widget->lightColor);
            shader.setVec3("lightPos", m_win_widget->lightPos);
            shader.setVec3("viewPos", m_win_widget->cameraPos);
        }
    }
}
