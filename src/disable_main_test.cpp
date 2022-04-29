#include "utils.hpp"
using namespace std;

#define ALMOST_ONE 0.99999999

NURBS_Surface face2, face3;

int main() {
    handleInput("static/face-1.txt", face2);
    handleInput("static/face-4.txt", face3);
    vector<pair<double, double>> uv_edge = {
        // left edge
        make_pair(0.0, 0.0),
        make_pair(0.25, 0.0),
        make_pair(0.5, 0.0),
        make_pair(0.75, 0.0),
        make_pair(ALMOST_ONE, 0.0),
        // bottom edge
        make_pair(ALMOST_ONE, 0.25),
        make_pair(ALMOST_ONE, 0.5),
        make_pair(ALMOST_ONE, 0.75),
        make_pair(ALMOST_ONE, ALMOST_ONE),
        // right edge
        make_pair(0.75, ALMOST_ONE),
        make_pair(0.5, ALMOST_ONE),
        make_pair(0.25, ALMOST_ONE),
        make_pair(0.0, ALMOST_ONE),
        // top edge
        make_pair(0.0, 0.75),
        make_pair(0.0, 0.5),
        make_pair(0.0, 0.25)
    };
    int count = 0;
    for (auto& uv_iter : uv_edge) {
        if (count % 4 == 0) {
            cout << "第" << count / 4 + 1 << "条边的点集合：" << endl;
        }
        count ++;
        // iterator face2 and face3 point by getPntByUV and print
        Point f2p = getPntByUV(face2, uv_iter.first, uv_iter.second);
        Point f3p = getPntByUV(face3, uv_iter.first, uv_iter.second);
        printf("[%lf,%lf] %lf %lf %lf\n", uv_iter.first, uv_iter.second, f2p.x, f2p.y, f2p.z);
        printf("[%lf,%lf] %lf %lf %lf\n\n", uv_iter.first, uv_iter.second, f3p.x, f3p.y, f3p.z);
    }
    return 0;
}