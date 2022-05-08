#include <iostream>
#include <engine.h>
#include <renderer.h>
#include <thread>

#include <communication/AutoBus.hpp>

using namespace std;
using namespace ToothSpace;
using namespace RenderSpace;

int main() {
    int scale = 50;
    cout << "[scale] ";
    cin >> scale;

    thread work_thread([&]() {
        auto service = ToothEngine::get_instance()->create_service(".\\static", scale);
        service->refresh_edge();
        service->calculate_table("test.csv");
    });
    work_thread.detach();

    Renderer renderer(800, 600);

    return renderer.exec();
}
