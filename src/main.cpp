#include <iostream>
#include <thread>
#include "render/engine.h"
#include "tooth/engine.h"

#include <communication/AutoBus.hpp>

using namespace std;

int main() {
    thread logic_thread([&]() {
        int scale = 50;
        // cin >> scale;
        auto service = ToothSpace::make_service("static", scale);
        service->retag_point();
        service->calculate_table("test.csv");
    });
    logic_thread.detach();

    cout << "主线程ID: " << this_thread::get_id() << endl;
    auto renderer = RenderSpace::make_renderer(1200, 800);
    return renderer->exec();
}
