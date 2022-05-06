#include <iostream>
#include <surface.h>
#include <line.h>

using namespace std;

int main() {
    Surface a("static/face-1.txt");
    cout << a.get_normal_by_uv(0.94, 0.98) << "?" << endl;
    Ray r(Point(-1, 10, 0), Direction(1, 0, 0));
    Point p;
    cout << get_intersection(r, Point(0, 2, 1), Point(0, 0, 0), Point(0, 0, -1), p) << " " \
    << p << endl;
    return 0;
}