#include <iostream>
#include <nurbs.h>

using namespace std;

int main() {
    NURBSFace nurbs("static/face-1.txt", 100);
    nurbs.cache_points();
    cout << nurbs.get_point_by_uv(0, 0) << endl;

    return 0;
}
