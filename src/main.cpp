#include <iostream>
#include <nurbs.h>
#include <engine.h>
#include <printer.h>
#include <line.h>

using namespace std;
using namespace ToothSpace;

int main() {
    auto service = ToothEngine::get_instance()->create_service("static", 100);

    return 0;
}
