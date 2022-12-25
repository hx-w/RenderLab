#ifndef GLM_EXT_RAY_HPP
#define GLM_EXT_RAY_HPP

/**
 *  @author hx-w
 *  @brief ray methods using glm
 */


#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace glm_ext {
float distance_point_to_ray(const glm::vec3& p, const glm::vec3& ray_origin, const glm::vec3& ray_dir) {
    glm::vec3 v = p - ray_origin;
    float d = glm::length(v);
    float t = glm::dot(v, ray_dir);
    if (t <= 0) {
        return d;
    }
    float l2 = glm::dot(ray_dir, ray_dir);
    if (t >= l2) {
        return glm::length(p - (ray_origin + ray_dir));
    }
    return glm::length(v - t * ray_dir / l2);
}
}

#endif