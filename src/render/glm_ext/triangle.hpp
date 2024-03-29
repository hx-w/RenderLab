#pragma once
/**
 *  @author hx-w
 *  @brief triangle methods using glm
 */

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace glm_ext {
static inline float vec_angle(const glm::vec3& v1, const glm::vec3& v2) {
    float v1len = glm::length(v1);
    float v2len = glm::length(v2);
    if (v1len == 0.f || v2len == 0.f) {
        return 0.f;
    }
    return glm::acos(glm::dot(v1, v2) / (v1len * v2len));
}

static float triangle_area(const glm::vec3& a,
                           const glm::vec3& b,
                           const glm::vec3& c) {
#if 1
    glm::vec3 v12 = a - b;
    glm::vec3 v13 = a - c;
    glm::vec3 v23 = b - c;
    return glm::length(glm::cross(v12, v13)) / 2.f;
#else
    //应用海伦公式   S=1/4std::sqrt[(a+b+c)(a+b-c)(a+c-b)(b+c-a)]
	float lenA = std::sqrt(std::pow(b.x - c.x, 2) + std::pow(b.y - c.y, 2) + std::pow(b.z - c.z, 2));// b - c 两点的坐标
	float lenB = std::sqrt(std::pow(a.x - c.x, 2) + std::pow(a.y - c.y, 2) + std::pow(a.z - c.z, 2));// a - c 两点的坐标
	float lenC = std::sqrt(std::pow(b.x - a.x, 2) + std::pow(b.y - a.y, 2) + std::pow(b.z - a.z, 2));// a - b 两点的坐标
	float area = 1.0 / 4.0 * std::sqrt((lenA + lenB + lenC) * (lenA + lenB - lenC) * (lenA + lenC - lenB) * (lenB + lenC - lenA));
    return area;
#endif
}

static bool is_same_side(const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& v3, const glm::vec3& p) {
    glm::vec3 AB = v2 - v1;
    glm::vec3 AC = v3 - v1;
    glm::vec3 AP = p - v1;

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
        return (b + c) / 2.0f;
	}
	return p;
}

}  // namespace glm_ext