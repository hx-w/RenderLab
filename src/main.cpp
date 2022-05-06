#include <iostream>
#include "libs/coords.h"

using namespace std;

int main() {
    Coords pt(1.2, 2.0, 3.0);
    cout << pt.normalize() << endl;
    return 0;
}