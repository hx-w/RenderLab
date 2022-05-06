#include "surface.h"
#include "line.h"
#include <cstdio>

using namespace std;

Surface::Surface(const string& filename) {
    read_file(filename);
    // calc knot vector
    for (int i = 0; i < m_degree.first + 1; ++i) {
        m_u_knots.push_back(vector<Scalar>());
        calc_knot_vector(i, true);
    }
    for (int i = 0; i < m_degree.second + 1; ++i) {
        m_v_knots.push_back(vector<Scalar>());
        calc_knot_vector(i, false);
    }
}

Surface::~Surface() {
    m_control_points.clear();
    m_weights.clear();
    m_u_knots.clear();
    m_v_knots.clear();
}

void Surface::read_file(const string& filename) {
    FILE *infile = NULL;
    if ((infile = fopen(filename.c_str(), "r")) == NULL) {
        printf("Error: cannot open file %s\n", filename.c_str());
        exit(1);
    }
    double x, y, z;
    fscanf(infile, "%d %d %d %d", &m_degree.first, &m_degree.second, &m_order.first, &m_order.second);
    // read weights
    for (int i = 0; i < m_degree.first + 1; ++i) {
        vector<Scalar> temp;
        for (int j = 0; j < m_degree.second + 1; ++j) {
            fscanf(infile, "%lf", &x);
            temp.push_back(x);
        }
        m_weights.push_back(temp);
    }
    // read pnts
    for (int i = 0; i < m_degree.first + 1; ++i) {
        vector<Point> temp;
        for (int j = 0; j < m_degree.second + 1; ++j) {
            fscanf(infile, "%lf %lf %lf", &x, &y, &z);
            temp.push_back(Point(x, y, z));
        }
        m_control_points.push_back(temp);
    }
    fclose(infile);
}


void Surface::calc_knot_vector(int index, bool is_u) {
    vector<Scalar>& knots = is_u ? m_u_knots[index] : m_v_knots[index];
    const int degree = is_u ? m_degree.second : m_degree.first;
    const int order = is_u ? m_order.first : m_order.second;
    knots.clear();

    for (int i = 0; i <= order + degree + 1; ++i) {
        if (i <= order) knots.push_back(0.0);
        else if (i >= degree + 1 && i <= order + degree + 1) knots.push_back(1.0);
        else if (i >= order + 1 && i <= degree) {
            double sum = 0.0;
            for(int j = order + 1; j <= i; ++j) { 
                double numerator = 0.0;
                for (int loop = j - order; loop <= j - 1; ++loop) {
                    const Point& pnt = is_u ? m_control_points[index][loop] : m_control_points[loop][index];
                    const Point& pnt_pre = is_u ? m_control_points[index][loop - 1] : m_control_points[loop - 1][index];
                    numerator += pow(pnt.x() - pnt_pre.x(), 2) + pow(pnt.y() - pnt_pre.y(), 2);
                }
                double denominator = 0.0;
                for (int loop1 = order + 1; loop1 <= degree + 1; ++loop1) {
                    for (int loop2 = loop1 - order; loop2 <= loop1 - 1; ++loop2) {
                        const Point& pnt = is_u ? m_control_points[index][loop2] : m_control_points[loop2][index];
                        const Point& pnt_pre = is_u ? m_control_points[index][loop2 - 1] : m_control_points[loop2 - 1][index];
                        denominator += pow(pnt.x() - pnt_pre.x(), 2) + pow(pnt.y() - pnt_pre.y(), 2);
                    }
                }
                if (almostZero(denominator)) {
                    sum = 0.0;
                }
                else {
                    sum += numerator / denominator;
                }
            }
            knots.push_back(sum);
        }
        else {
            printf("error: calc_knot_vector\n");
        }
    }
}

Scalar Surface::basis_function_value(
    Scalar uv_value,
    int index,
    int order,
    const vector<Scalar>& knots
) const {
	double value1, value2, value;
	if (order == 0) {
		if (uv_value >= knots[index] && uv_value < knots[index + 1])
			return 1.0;
		else
			return 0.0;
	}
	if (order > 0) {
		if (uv_value < knots[index] || uv_value > knots[index + order + 1]) {
			return 0.0;
        }
		else {
			double coffcient1, coffcient2;//凸组合系数1，凸组合系数2
			double denominator = 0.0;//分母
			denominator = knots[index + order] - knots[index];//递推公式第一项分母
			if (denominator == 0.0) //约定0/0
				coffcient1 = 0.0;
			else
				coffcient1=(uv_value - knots[index]) / denominator;
			denominator = knots[index + order + 1] - knots[index + 1]; //递推公式第二项分母
			if (0.0 == denominator)//约定0/0
				coffcient2 = 0.0;
			else
				coffcient2 = (knots[index + order + 1] - uv_value) / denominator;
			value1 = coffcient1 * basis_function_value(uv_value, index, order - 1, knots);//递推公式第一项的值
			value2 = coffcient2 * basis_function_value(uv_value, index + 1, order - 1, knots);//递推公式第二项的值
			value = value1 + value2;//基函数的值
		}
	}
	return value;
}

Point Surface::get_point_by_uv(Scalar u, Scalar v) const {
    Scalar weight = 0.0;
    Point pnt(0.0, 0.0, 0.0);
    for (int i = 0; i < m_degree.first + 1; ++i) {
        for (int j = 0; j < m_degree.second + 1; ++j) {
            Scalar basis_u = basis_function_value(u, j, m_order.first, m_u_knots[i]);
            Scalar basis_v = basis_function_value(v, i, m_order.second, m_v_knots[j]);
            pnt += m_control_points[i][j] * m_weights[i][j] * basis_u * basis_v;
            weight += m_weights[i][j] * basis_u * basis_v;
        }
    }
    if (almostZero(weight)) {
        return Point(0.0, 0.0, 0.0);
    }
    pnt /= weight;
    return pnt;
}

Direction Surface::get_normal_by_uv(Scalar u, Scalar v, Scalar delta) const {
    Point pmid = get_point_by_uv(u, v);
    Point pl = get_point_by_uv(u - delta, v);
    Point pr = get_point_by_uv(u + delta, v);
    Point pu = get_point_by_uv(u, v - delta);
    Point pd = get_point_by_uv(u, v + delta);

#ifdef SIMPLE_NORMAL
    return (pr - pl).cross(pd - pu).normalize();
#else
    return (
        get_normal(pmid, pl, pu) + \
        get_normal(pmid, pu, pr) + \
        get_normal(pmid, pr, pd) + \
        get_normal(pmid, pd, pl)
    ).normalize();
#endif
}