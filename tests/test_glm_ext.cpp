#include <iostream>
#include "../src/render/glm_ext/curvature.hpp"
using namespace std;
using namespace glm_ext;

typedef glm::vec3 Point3d;

Point3d triangle_circumcircle_center_template(Point3d a, Point3d b, Point3d c) {
    float a1, b1, c1, d1;
    float a2, b2, c2, d2;
    float a3, b3, c3, d3;

    float x1 = a.x, y1 = a.y, z1 = a.z;
    float x2 = b.x, y2 = b.y, z2 = b.z;
    float x3 = c.x, y3 = c.y, z3 = c.z;

    a1 = (y1 * z2 - y2 * z1 - y1 * z3 + y3 * z1 + y2 * z3 - y3 * z2);
    b1 = -(x1 * z2 - x2 * z1 - x1 * z3 + x3 * z1 + x2 * z3 - x3 * z2);
    c1 = (x1 * y2 - x2 * y1 - x1 * y3 + x3 * y1 + x2 * y3 - x3 * y2);
    d1 = -(x1 * y2 * z3 - x1 * y3 * z2 - x2 * y1 * z3 + x2 * y3 * z1 +
           x3 * y1 * z2 - x3 * y2 * z1);

    a2 = 2 * (x2 - x1);
    b2 = 2 * (y2 - y1);
    c2 = 2 * (z2 - z1);
    d2 = x1 * x1 + y1 * y1 + z1 * z1 - x2 * x2 - y2 * y2 - z2 * z2;

    a3 = 2 * (x3 - x1);
    b3 = 2 * (y3 - y1);
    c3 = 2 * (z3 - z1);
    d3 = x1 * x1 + y1 * y1 + z1 * z1 - x3 * x3 - y3 * y3 - z3 * z3;
    Point3d rlt;
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

float triangle_area_template(Point3d a, Point3d b, Point3d c) {
	//应用海伦公式   S=1/4sqrt[(a+b+c)(a+b-c)(a+c-b)(b+c-a)]
	float lenA = sqrt(pow(b.x - c.x, 2) + pow(b.y - c.y, 2) + pow(b.z - c.z, 2));// b - c 两点的坐标
	float lenB = sqrt(pow(a.x - c.x, 2) + pow(a.y - c.y, 2) + pow(a.z - c.z, 2));// a - c 两点的坐标
	float lenC = sqrt(pow(b.x - a.x, 2) + pow(b.y - a.y, 2) + pow(b.z - a.z, 2));// a - b 两点的坐标
	float Area = 1.0 / 4.0 * sqrt((lenA + lenB + lenC) * (lenA + lenB - lenC) * (lenA + lenC - lenB) * (lenB + lenC - lenA));
	return Area;
}

static void print_vec3(const glm::vec3& v) {
    cout << "(" << v.x << ", " << v.y << ", " << v.z << ")" << endl;
}

int main() {
    //! glm_ext/triangle.hpp
    {
        glm::vec3 v1(4, 5, 0);
        glm::vec3 v2(3, 2, 0);
        glm::vec3 v3(9, 2, 0);

        glm::vec3 v4(7, 4, 2);

        // 三角形外心
        Point3d my = triangle_circumcircle_center(v1, v2, v3);
        Point3d true_template = triangle_circumcircle_center_template(v1, v2, v3);
        assert(my == true_template);

        // 三角形面积
        float area = triangle_area(v1, v2, v3);
        float area_template = triangle_area_template(v1, v2, v3);
        assert(fabs(area - area_template) < 1e-6);
    }

    //! glm_ext/curvature.hpp
    {
        Point3d p(-6.13622999, -61.2943993, -14.3997002);
        Point3d n1(-6.56057978, -61.4015007, -15.0757999);
        Point3d n2(-5.87774992, -61.8502998, -14.8935003);
        Point3d n3(-5.59214020, -61.4595985, -13.9681997);
        // 高斯曲率 (p)
        vector<glm::vec3> nebs { n1, n2, n3 };
        float k = compute_curvature(p, nebs, glm_ext::CURVATURE_GAUSSIAN);
        cout << "Gaussian curvature: " << k << endl;

        // 高斯曲率部分
        float coff = curvature_Guassian(p, n1, n2);
        cout << "coff: " << coff << endl;
    }
    cout << "test passed" << endl;
    return 0;
}