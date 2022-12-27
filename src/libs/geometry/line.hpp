/**
 *  methods with line
 */

#include <glm/glm.hpp>
#include "geom_types.h"

namespace geometry {
    class Line {
    public:
        Line() = default;
        Line(const Point3f& p1, const Point3f& p2) : m_p1(p1), m_p2(p2) {}

        Point3f get_p1() const { return m_p1; }
        Point3f get_p2() const { return m_p2; }
        Point3f& p1() { return m_p1; }
        Point3f& p2() { return m_p2; }

        Point3f get_midpoint() const { return (m_p1 + m_p2) / 2.f; }

        float get_length() const { return glm::length(m_p2 - m_p1); }

        Vector3f get_direction() const { return glm::normalize(m_p2 - m_p1); }

        Point3f get_point(float t) const { return m_p1 + t * (m_p2 - m_p1); }

        float get_distance(const Point3f& p) const {
            return glm::length(glm::cross(m_p2 - m_p1, m_p1 - p)) / get_length();
        }

        float get_distance(const Line& line) const {
            return glm::length(glm::cross(m_p2 - m_p1, line.m_p1 - m_p1)) / get_length();
        }

    private:
        Point3f m_p1;
        Point3f m_p2;
    };

    class Ray {
    public:
        Ray() = default;
        Ray(const Point3f& origin, const Vector3f& direction)
            : m_origin(origin), m_direction(direction) {}

        Point3f get_origin() const { return m_origin; }
        Vector3f get_direction() const { return m_direction; }
        Point3f& origin() { return m_origin; }
        Vector3f& direction() { return m_direction; }

        Point3f get_point(float t) const { return m_origin + t * m_direction; }

        float get_distance(const Point3f& p) const {
            return glm::length(glm::cross(m_direction, m_origin - p));
        }

    private:
        Point3f m_origin;
        Vector3f m_direction;
    };
}