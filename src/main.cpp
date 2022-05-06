#include <iostream>
#include <coords.h>

using namespace std;

int main() {
    Coords pt(1.2, 2.0, 3.0);
    cout << pt.hash() << endl;
    return 0;
}