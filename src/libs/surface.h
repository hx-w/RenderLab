#ifndef SURFACE_H
#define SURFACE_H

#include <vector>
#include <string>
#include "coords.h"

using std::vector;
typedef std::pair<int, int> UV;
typedef vector<vector<Scalar>> Matrix_double;
typedef vector<vector<Point>> Matrix_point;

// nurbs surface
class Surface {
public:
    Surface() = delete;
    Surface(const std::string& filename);
    ~Surface();

    Point get_point_by_uv(Scalar u, Scalar v) const;
    Direction get_normal_by_uv(Scalar u, Scalar v, Scalar delta=0.005) const;

private:
    void read_file(const std::string& filename);
    void calc_knot_vector(int index, bool is_u);
    Scalar basis_function_value(
        Scalar uv_value,
        int index,
        int order,
        const vector<Scalar>& knots
    ) const;

private:
    UV m_order;
    UV m_degree;   // uv 两个方向控制点的个数
    Matrix_point m_control_points;
    Matrix_double m_weights;
    Matrix_double m_u_knots;
    Matrix_double m_v_knots;
};

#endif
