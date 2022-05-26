#include <iostream>
#include <thread>
#include "render/engine.h"
#include "tooth/engine.h"
#include "tooth/execute.h"

using namespace std;

int main() {
    cout << "main thread ID: " << this_thread::get_id() << endl;
    auto renderer = RenderSpace::make_renderer(1200, 800);

    thread ml_thread([&]() {
        execute("python3 scripts/ml_server.py");
    });
    ml_thread.detach();

    thread logic_thread([&]() {
        std::this_thread::sleep_for(800ms);
        int scale = 100;  // uv方向采样次数
        // 对应static/中的文件夹，每个文件夹包含一颗牙的四个表面
        // auto source_list = {"N1", "N2", "N3", "N4", "N5"};
        auto source_list = {"N6", "N7", "N8", "N9", "N10"};
        for (auto& source : source_list) {
            auto service = ToothSpace::make_service(
                "static/nurbs/" + string(source), scale);
            service->retag_point_by_ml();
            service->simulate_by_ml();
        }
    });
    logic_thread.detach();

    return renderer->exec();
}