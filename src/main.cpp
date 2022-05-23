#include <iostream>
#include <thread>
#include "render/engine.h"
#include "tooth/engine.h"

using namespace std;

int main() {
    cout << "main thread ID: " << this_thread::get_id() << endl;

    auto renderer = RenderSpace::make_renderer(1200, 800);
    thread logic_thread([&]() {
        int scale = 100;  // uv方向采样次数
        // 对应static/中的文件夹，每个文件夹包含一颗牙的四个表面
        auto source_list = { "sample", "N1", "N2" };
        // auto source_list = {
        //     "N1", "N2", "N3", "N4", "N5",
        //     "N6", "N7", "N8", "N9", "N10",
        // };
        for (auto& source : source_list) {
            auto service = ToothSpace::make_service(
                "static/nurbs/" + string(source), scale
            );
            service->retag_point();
            service->simulate("test.csv");
        }
    });
    logic_thread.detach();

    return renderer->exec();
}
