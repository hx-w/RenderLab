#include <iostream>
#include <coords.h>

using namespace std;

int main() {
    Locate pt(1, 10, 3);
    cout << pt.hash() << endl;
    return 0;
}