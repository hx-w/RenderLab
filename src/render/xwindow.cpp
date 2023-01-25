#include "xwindow.h"
#include "service.h"
#include "context.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <chrono>

using namespace std;

namespace RenderSpace {
    time_t get_timestamp() {
        auto tp = chrono::time_point_cast<chrono::milliseconds>(chrono::system_clock::now());
        return tp.time_since_epoch().count();
    }

    void RenderWindowWidget::init_context(std::shared_ptr <RenderContext> ctx) {
        m_context = ctx;
        init(m_context->service());
    }

    void RenderWindowWidget::init(shared_ptr<RenderService> service) {
        m_service = service;

        auto _keys = {
            GLFW_KEY_H, GLFW_KEY_T, GLFW_KEY_LEFT_CONTROL, GLFW_KEY_R,
            GLFW_KEY_DELETE, GLFW_KEY_BACKSPACE
        };

        for (auto _k : _keys) {
            m_key_last_active[_k] = 0;
        }

        imguiGizmo::setGizmoFeelingRot(1.2f);
    }

    RenderWindowWidget::~RenderWindowWidget() {
    }


    // process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
    // ---------------------------------------------------------------------------------------------------------
    void RenderWindowWidget::processInput(GLFWwindow* window) {
        for (auto& [_k, _tp] : m_key_last_active) {
            if (glfwGetKey(window, _k) == GLFW_PRESS) {
                _tp = get_timestamp();
            }
            if (glfwGetKey(window, _k) == GLFW_RELEASE) {
                // check press duration time [ <= 700ms ]
                if (get_timestamp() - _tp <= 700) {
                    /// [Notify]
                    m_service->notify<void(int)>("/keyboard_clicked", _k);
                    _tp = 0;
                }
            }
        }

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        float cameraMove = cameraSpeed * deltaTime;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cameraPos += cameraMove * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cameraPos -= cameraMove * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraMove;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraMove;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            cameraPos -= cameraMove * glm::vec3(0.0f, 1.0f, 0.0f);
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            cameraPos += cameraMove * glm::vec3(0.0f, 1.0f, 0.0f);
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            key_down_GTRL = true;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE) {
            key_down_GTRL = false;
        }
    }

    // glfw: whenever the window size changed (by OS or user resize) this callback function executes
    // ---------------------------------------------------------------------------------------------
    void RenderWindowWidget::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        // make sure the viewport matches the new window dimensions; note that width and 
        // height will be significantly larger than specified on retina displays.
        m_scr_width = width;
        m_scr_height = height;
        glViewport(0, 0, m_scr_width, m_scr_height);
        /// [TODO] window resize notify

        gizmo.viewportSize(width, height);
        gizmo.setDollyScale(1.0f / (width > height ? height : width));
        gizmo.setPanScale(1.0f / (width > height ? height : width));
    }

    // glfw: whenever the mouse moves, this callback is called
    // -------------------------------------------------------
    void RenderWindowWidget::mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
        float xpos = static_cast<float>(xposIn);
        float ypos = static_cast<float>(yposIn);
        realX = xpos;
        realY = ypos;
        if (!leftMousePressed)
            return;

        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
        lastX = xpos;
        lastY = ypos;

        float sensitivity = 0.1f; // change this value to your liking
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        glm::vec3 front(
            cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
            sin(glm::radians(pitch)),
            sin(glm::radians(yaw)) * cos(glm::radians(pitch))
        );
        cameraFront = glm::normalize(front);
    }

    // glfw: whenever the mouse scroll wheel scrolls, this callback is called
    // ----------------------------------------------------------------------
    void RenderWindowWidget::scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
        fov -= (float)yoffset;
        if (fov < 1.0f)
            fov = 1.0f;
        if (fov > 45.0f)
            fov = 45.0f;
    }

    void RenderWindowWidget::mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
        auto get_modifier = [this](GLFWwindow* window) {
            if ((glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS))
                return GLFW_MOD_CONTROL;
            else if ((glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS))
                return GLFW_MOD_SHIFT;
            else if ((glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS))
                return GLFW_MOD_ALT;
            return 0;
        };

        gizmo.mouse((vgButtons)(button), get_modifier(window), action == GLFW_PRESS, realX, realY);
        
        /// [Notify] render/mouse_event
        m_service->notify<void(int, int)>("/mouse_event", button, action);

	    if (action == GLFW_PRESS) {
            switch(button) {
			case GLFW_MOUSE_BUTTON_LEFT:
                leftMousePressed = true;
                firstMouse = true;
				break;
			case GLFW_MOUSE_BUTTON_MIDDLE:
				break;
			case GLFW_MOUSE_BUTTON_RIGHT:
				break;
			default:
				return;
			}
        }
        if (action == GLFW_RELEASE) {
            switch(button) {
			case GLFW_MOUSE_BUTTON_LEFT:
                leftMousePressed = false;
                if (interact_mode == PickMode && key_down_GTRL) {
                    glm::vec3 direction(0.0);
                    pickingRay(glm::vec2(realX, realY), direction);
                    /// [TODO] picking ray notify
                    m_context->ctx_pick_drawables(cameraPos, direction, false);
                }
				break;
			case GLFW_MOUSE_BUTTON_MIDDLE:
				break;
			case GLFW_MOUSE_BUTTON_RIGHT:
				break;
			default:
				return;
			}
        }
    }

    void RenderWindowWidget::dropfile_callback(GLFWwindow* window, int count, const char** paths) {
        for (int i = 0; i < count; ++i) {
            // recive files dropin
            auto path = string(paths[i]);
            m_service->notify<void(const string&)>("/filepath_dropin", path);
        }
    }

    void RenderWindowWidget::pickingRay(glm::vec2 screen_pos, glm::vec3& direction) {
        // implement screen to world transform and picking ray generation
        glm::vec3 origin(0.0);
        screen2world(screen_pos, origin);
        direction = origin - cameraPos;
    }

    void RenderWindowWidget::screen2world(glm::vec2 screen_pos, glm::vec3& world_pos) {
        // implement screen to world transform
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)m_scr_width / (float)m_scr_height, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 mvp = projection * view;
        glm::mat4 inv_mvp = glm::inverse(mvp);
        // convert to NDC space
        glm::vec4 v_NDC(1.0);
        v_NDC.x = (2.0f * screen_pos.x) / m_scr_width - 1.0f;
        v_NDC.y = 1.0f - 2.0f * (screen_pos.y / m_scr_height);
        glm::vec4 v_world = inv_mvp * v_NDC;
        v_world.w = 1.0 / v_world.w;
        world_pos = glm::vec3(v_world.x * v_world.w, v_world.y * v_world.w, v_world.z * v_world.w);
    }

    void RenderWindowWidget::viewfit_BBOX(const AABB& aabb) {
        glm::vec3 center = (aabb.first + aabb.second) / 2.0f;
        glm::vec3 size = aabb.second - aabb.first;
        float max_size = std::max(std::max(size.x, size.y), size.z);
        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraPos = center - front * max_size * 1.5f;
        cameraFront = front;
    }

    void RenderWindowWidget::set_interact_mode(InteractMode mode) {
        interact_mode = mode;
    }

    void RenderWindowWidget::update_transform_mat(const mat4& transf) {
        cameraPos = -glm::vec3(transf[3][1], transf[3][0], transf[3][2] + 20.f);
        gizmo.setRotationCenter(glm::vec3(transf[3][1], transf[3][0], -transf[3][2]));
    }
}
