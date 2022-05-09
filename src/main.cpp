#include <iostream>
#include <engine.h>
#include <renderer.h>
#include <thread>

#include <communication/AutoBus.hpp>

using namespace std;
using namespace RenderSpace;

int main() {
    thread logic_thread([&]() {
        int scale = 100;
        auto service = ToothSpace::make_service("static", scale);
        service->refresh_edge();
        service->calculate_table("test.csv");
    });
    logic_thread.detach();

    Renderer renderer(800, 600);

    return renderer.exec();
}
