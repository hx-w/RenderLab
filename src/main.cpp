#include <iostream>
#include <nurbs.h>
#include <engine.h>
#include <printer.h>

using namespace std;
using namespace ToothSpace;

int main() {
    auto engine = ToothSpace::ToothEngine::get_instance();
    NURBSFace nurbs("static/face-1.txt", 100);
    nurbs.cache_points();
    cout << nurbs.get_point_by_uv(0, 0) << endl;

    auto printer = Printer("test.csv");

    printer.to_csv("1", 12, "2", "3");
    printer.to_console("ddd", 123);

    return 0;
}
