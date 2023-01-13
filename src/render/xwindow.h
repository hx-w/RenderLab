#ifndef RENDER_WINDOW_H
#define RENDER_WINDOW_H

#include <memory>
#include <map>
#include <time.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct GLFWwindow;
namespace RenderSpace {
    typedef std::pair<glm::vec3, glm::vec3> AABB; // min, max

    class RenderService;
    class RenderContext;
    class RenderWindowWidget {
    public:
        RenderWindowWidget() = default;
        ~RenderWindowWidget();
        RenderWindowWidget(const RenderWindowWidget&) = delete;

        void init_context(std::shared_ptr<RenderContext> ctx);
        void init(std::shared_ptr<RenderService> service);

    public:
        void framebuffer_size_callback(GLFWwindow* window, int width, int height);
        void mouse_callback(GLFWwindow* window, double xpos, double ypos);
        void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
        void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
        void processInput(GLFWwindow* window);

        void pickingRay(glm::vec2 screen_pos, glm::vec3& direction);
        void screen2world(glm::vec2 screen_pos, glm::vec3& world_pos);

        // view control
        void viewfit_BBOX(const AABB& aabb);

    public:
        unsigned int m_scr_width = 800;
        unsigned int m_scr_height = 600;

        // camera
        glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f, 10.0f);
        glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f, 0.0f);
        float cameraSpeed = 8.f;

        bool leftMousePressed = false;
        bool firstMouse = true;
        float yaw   = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
        float pitch =  0.0f;
        float lastX =  800.0f / 2.0;
        float lastY =  600.0 / 2.0;
        float fov   =  45.0f;

        float deltaTime = 0.0f;	// time between current frame and last frame
        float lastFrame = 0.0f;

        std::shared_ptr<RenderService> m_service;
        bool all_visible = true;
        glm::vec4 bgColor = glm::vec4(0.1f, 0.1f, 0.1f, 1.00f);

        float realX = 0.0f;
        float realY = 0.0f;

        // light
        glm::vec3 lightPos = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
   

    private:
        std::map<int, time_t> m_key_last_active; // for click, only record GLFW_PRESS

    private:
        std::shared_ptr<RenderContext> m_context;
    };
}

#endif
