#include <iostream>
#include <nurbs.h>
#include <engine.h>

using namespace std;

int main() {
    auto engine = ToothSpace::ToothEngine::get_instance();
    NURBSFace nurbs("static/face-1.txt", 100);
    nurbs.cache_points();
    cout << nurbs.get_point_by_uv(0, 0) << endl;

    return 0;
}
