#include <iostream>
#include <engine.h>
#include <renderer.h>

using namespace std;
using namespace ToothSpace;
using namespace RenderSpace;

int main() {
    int scale = 50;
    cout << "[scale] ";
    cin >> scale;
    auto service = ToothEngine::get_instance()->create_service(".\\static", scale);

    service->refresh_edge();
    service->calculate_table("test.csv");

    Renderer renderer(800, 600);

    return renderer.exec();
}
