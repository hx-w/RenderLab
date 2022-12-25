#include "xwindow.h"
#include "service.h"
#include <iostream>

using namespace std;

namespace RenderSpace {
    RenderWindowWidget::RenderWindowWidget(unsigned int width, unsigned int height, shared_ptr<RenderService> service) {
        init(width, height, service);
    }

    void RenderWindowWidget::init(unsigned int width, unsigned int height, shared_ptr<RenderService> service) {
        m_scr_width = width;
        m_scr_height = height;
        m_service = service;
        lastX = width * 1.0 / 2;
        lastY = height * 1.0 / 2;

        m_service->notify_window_resize(width, height);
    }

    RenderWindowWidget::~RenderWindowWidget() {
    }


    // process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
    // ---------------------------------------------------------------------------------------------------------
    void RenderWindowWidget::processInput(GLFWwindow* window) {
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
        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
            if (!H_down) {
                H_down = !H_down;
                show_gui = !show_gui;
            } 
        }
        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE) {
            H_down = false;
        }
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
            if (!T_down) {
                T_down = !T_down;
                T_EventHandler();
            } 
        }
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) {
            T_down = false;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
            CTRL_down = true;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE) {
            CTRL_down = false;
        }
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            if (!R_down) {
                R_down = !R_down;
                if (CTRL_down) {
                    m_service->notify_clear_picking();
                }
            }
        }
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
            R_down = false;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            cameraPos -= cameraMove * glm::vec3(0.0f, 1.0f, 0.0f);
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            cameraPos += cameraMove * glm::vec3(0.0f, 1.0f, 0.0f);
        }
    }

    // glfw: whenever the window size changed (by OS or user resize) this callback function executes
    // ---------------------------------------------------------------------------------------------
    void RenderWindowWidget::framebuffer_size_callback(GLFWwindow* window, int width, int height) {
        // make sure the viewport matches the new window dimensions; note that width and 
        // height will be significantly larger than specified on retina displays.
        m_scr_width = width;
        m_scr_height = height;
        m_service->notify_window_resize(m_scr_width, m_scr_height);
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

        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
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
                if (CTRL_down) {
                    glm::vec3 direction(0.0);
                    pickingRay(glm::vec2(realX, realY), direction);
                    m_service->ray_pick(cameraPos, direction);
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

    void RenderWindowWidget::T_EventHandler() {
        // TODO
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
}
