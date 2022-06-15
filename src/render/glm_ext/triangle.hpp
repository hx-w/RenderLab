#pragma once
/**
 *  @author hx-w
 *  @brief triangle methods using glm
 */

#include "../libs/glm/glm.hpp"
#include "../libs/glm/gtc/matrix_transform.hpp"
#include "../libs/glm/gtc/type_ptr.hpp"

namespace glm_ext {
static inline float vec_angle(const glm::vec3& v1, const glm::vec3& v2) {
    float v1len = glm::length(v1);
    float v2len = glm::length(v2);
    if (v1len == 0.f || v2len == 0.f) {
        return 0.f;
    }
    return glm::acos(glm::dot(v1, v2) / (v1len * v2len));
}

static float triangle_area(const glm::vec3& v1,
                           const glm::vec3& v2,
                           const glm::vec3& v3) {
    glm::vec3 v12 = v1 - v2;
    glm::vec3 v13 = v1 - v3;
    glm::vec3 v23 = v2 - v3;
    return glm::length(glm::cross(v12, v13)) / 2.f;
}

static bool is_same_side(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec3& p) {
    glm::vec3 AB(v2.x - v1.x, v2.y - v1.y, v2.z - v1.z);
    glm::vec3 AC(v3.x - v1.x, v3.y - v1.y, v3.z - v1.z);
    glm::vec3 AP(p.x - v1.x, p.y - v1.y, p.z - v1.z);

    glm::vec3 vx1 = glm::cross(AB, AC);
    glm::vec3 vx2 = glm::cross(AB, AP);

    return glm::dot(vx1, vx2) >= 0;
}

// @brief 判断点是否在三角形内
static bool is_in_triangle(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& p) {
    return is_same_side(a, b, c, p) && is_same_side(b, c, a, p) && is_same_side(c, a, b, p);
}

// 三角形外接圆圆心
static glm::vec3 triangle_circumcircle_center(const glm::vec3& v1,
                                              const glm::vec3& v2,
                                              const glm::vec3& v3) {
    float x1 = v1.x, y1 = v1.y, z1 = v1.z;
    float x2 = v2.x, y2 = v2.y, z2 = v2.z;
    float x3 = v3.x, y3 = v3.y, z3 = v3.z;

    float a1 = (y1 * z2 - y2 * z1 - y1 * z3 + y3 * z1 + y2 * z3 - y3 * z2);
    float b1 = -(x1 * z2 - x2 * z1 - x1 * z3 + x3 * z1 + x2 * z3 - x3 * z2);
    float c1 = (x1 * y2 - x2 * y1 - x1 * y3 + x3 * y1 + x2 * y3 - x3 * y2);
    float d1 = -(x1 * y2 * z3 - x1 * y3 * z2 - x2 * y1 * z3 + x2 * y3 * z1 +
                 x3 * y1 * z2 - x3 * y2 * z1);

    float a2 = 2 * (x2 - x1);
    float b2 = 2 * (y2 - y1);
    float c2 = 2 * (z2 - z1);
    float d2 = x1 * x1 + y1 * y1 + z1 * z1 - x2 * x2 - y2 * y2 - z2 * z2;

    float a3 = 2 * (x3 - x1);
    float b3 = 2 * (y3 - y1);
    float c3 = 2 * (z3 - z1);
    float d3 = x1 * x1 + y1 * y1 + z1 * z1 - x3 * x3 - y3 * y3 - z3 * z3;
    glm::vec3 rlt = glm::vec3(0.f, 0.f, 0.f);
    rlt.x = -(b1 * c2 * d3 - b1 * c3 * d2 - b2 * c1 * d3 + b2 * c3 * d1 +
              b3 * c1 * d2 - b3 * c2 * d1) /
            (a1 * b2 * c3 - a1 * b3 * c2 - a2 * b1 * c3 + a2 * b3 * c1 +
             a3 * b1 * c2 - a3 * b2 * c1);
    rlt.y = (a1 * c2 * d3 - a1 * c3 * d2 - a2 * c1 * d3 + a2 * c3 * d1 +
             a3 * c1 * d2 - a3 * c2 * d1) /
            (a1 * b2 * c3 - a1 * b3 * c2 - a2 * b1 * c3 + a2 * b3 * c1 +
             a3 * b1 * c2 - a3 * b2 * c1);
    rlt.z = -(a1 * b2 * d3 - a1 * b3 * d2 - a2 * b1 * d3 + a2 * b3 * d1 +
              a3 * b1 * d2 - a3 * b2 * d1) /
            (a1 * b2 * c3 - a1 * b3 * c2 - a2 * b1 * c3 + a2 * b3 * c1 +
             a3 * b1 * c2 - a3 * b2 * c1);

    return rlt;
}

// @brief 根据外接圆的圆心是否要修正，做合理的修正
static glm::vec3 VoronoiMixed_center(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
	glm::vec3 p = triangle_circumcircle_center(a, b, c);//计算外接圆的圆心
	if (!is_in_triangle(a, b, c, p)) {//不在三角形内部的时候 返回bc的中点
		return { (b.x + c.x) / 2.0, (b.y + c.y) / 2.0, (b.z + c.z) / 2.0 };
	}
	return p;
}

}  // namespace glm_ext