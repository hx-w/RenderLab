#include <iostream>
#include <nurbs.h>
#include <engine.h>
#include <printer.h>

using namespace std;
using namespace ToothSpace;

int main() {
    int scale = 100;
    Printer::to_console("[scale] ");
    cin >> scale;
    auto service = ToothEngine::get_instance()->create_service("static", scale);

    service->refresh_edge();
    service->calculate_table("test.csv");

#ifdef __WIN32__
    system("pause");
#endif
    return 0;
}
