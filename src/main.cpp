#include <iostream>
#include <nurbs.h>
#include <engine.h>
#include <printer.h>

using namespace std;
using namespace ToothSpace;



int main() {
    auto service = ToothEngine::get_instance()->create_service("static", 100);

    service->refresh_edge();
    service->calculate_table("test.csv");

    return 0;
}
