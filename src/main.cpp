#include <iostream>
#include <surface.h>

using namespace std;

int main() {
    Surface a("static/face-1.txt");
    cout << a.get_point_by_uv(0.94, 0.98) << "?" << endl;
    return 0;
}