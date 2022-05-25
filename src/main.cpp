#include <iostream>
#include <thread>
#include "render/engine.h"
#include "tooth/engine.h"
#include "execute.hpp"

using namespace std;


int main() {
    cout << "main thread ID: " << this_thread::get_id() << endl;
    auto renderer = RenderSpace::make_renderer(1200, 800);

    thread ml_thread([&]() {
        execute("python3 scripts/ml_server.py");
    });
    ml_thread.detach();

    thread logic_thread([&]() {
        int scale = 100;  // uv方向采样次数
        // 对应static/中的文件夹，每个文件夹包含一颗牙的四个表面
        auto source_list = {"sample", "N1", "N2"};
        for (auto& source : source_list) {
            auto service = ToothSpace::make_service(
                "static/nurbs/" + string(source), scale);
            service->retag_point();
            service->simulate();
        }
    });
    logic_thread.detach();

    return renderer->exec();
}
