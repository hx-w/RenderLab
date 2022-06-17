#pragma once
/**
 *  @author hx-w
 *  @brief curvature methods using glm
 */

#include <vector>
#include "triangle.hpp"
#include "../libs/glm/gtx/vector_angle.hpp"

namespace glm_ext {

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

enum CurvatureType { CURVATURE_GAUSSIAN, CURVATURE_MEAN };

// 对于a-b-c三角形，计算voronoi点，并返回该三角形曲率系数(角度)
static float curvature_Guassian(const glm::vec3& a,
                                const glm::vec3& b,
                                const glm::vec3& c) {
    return glm::angle(glm::normalize(c - a), glm::normalize(b - a));
}

// 计算cotangent系数 得到长度
static float curvature_Mean(const glm::vec3& p,
                            const glm::vec3& neb_prev,
                            const glm::vec3& neb,
                            const glm::vec3& neb_next) {
    // compute alpha beta
    glm::vec3 AP = glm::normalize(p - neb_prev);
    glm::vec3 AB = glm::normalize(neb - neb_prev);
    float cosAlpha = glm::cos(glm::angle(AP, AB));

    if (cosAlpha >= 0.99f) {
        return 0.0f;
    }
    float cotAlpha = cosAlpha / glm::sqrt(1 - cosAlpha * cosAlpha);
    glm::vec3 CP = glm::normalize(p - neb_next);
    glm::vec3 CB = glm::normalize(neb - neb_next);
    float cosBeta = glm::cos(glm::angle(CP, CB));
    if (cosBeta >= 0.99f) {
        return 0.0f;
    }
    float cotBeta = cosBeta / glm::sqrt(1 - cosBeta * cosBeta);

    float len = glm::length(neb - p);
    float coff = (cotAlpha + cotBeta) * len * len;

    return coff;
}

// neighors 拓扑有序
static float compute_curvature(const glm::vec3& p,
                               const std::vector<glm::vec3>& neighbors,
                               CurvatureType type) {
    float curvature = 0.f;
    // 计算 Voronoi 点集
    std::vector<glm::vec3> voronoi_points;

    float coff = 0.f;
    size_t neb_size = neighbors.size();
    for (auto neb_idx = 0; neb_idx < neb_size; ++neb_idx) {
        auto& neb = neighbors[neb_idx];
        auto& neb_prev = neighbors[neb_idx == 0 ? neb_size - 1 : neb_idx - 1];
        auto& neb_next = neighbors[(neb_idx + 1) % neb_size];
        voronoi_points.emplace_back(VoronoiMixed_center(p, neb, neb_next));
        switch (type) {
            case CURVATURE_GAUSSIAN:
                coff += curvature_Guassian(p, neb, neb_next);
                break;
            case CURVATURE_MEAN:
                coff += curvature_Mean(p, neb_prev, neb, neb_next);
                break;
            default:
                break;
        }
    }

    // 计算Voronoi三角形面积和
    float sum_area = 0.f;
    auto voronoi_size = voronoi_points.size();
    for (auto i = 0; i < voronoi_size; ++i) {
        auto& v1 = voronoi_points[i];
        auto& v2 = voronoi_points[(i + 1) % voronoi_points.size()];
        sum_area += triangle_area(p, v1, v2);
    }

    if (sum_area <= 0.f) {
        return 0.0f;
    }
    switch (type) {
        case CURVATURE_GAUSSIAN:
            curvature = (1.0 / sum_area) * (2 * M_PI - coff);
            break;
        case CURVATURE_MEAN:
            curvature = 0.5f * (1.f / (sum_area * 2) * glm::sqrt(coff));
            break;
        default:
            break;
    }
    return curvature;
}
}  // namespace glm_ext
