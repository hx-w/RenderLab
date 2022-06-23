#include <iostream>
#include <thread>
#include "render/engine.h"
#include "xtooth/engine.h"
// #include "train_and_test.hpp"

using namespace std;

int main() {
    cout << "main thread ID: " << this_thread::get_id() << endl;
    auto renderer = RenderSpace::make_renderer(1800, 900);

    // thread ml_thread(train_and_test, 100, renderer->get_service());
    // ml_thread.detach();

    thread xtooth_thread([&](){
        auto xtooth = XToothSpace::make_service("xtooth");
        xtooth->simulate();
        });
    xtooth_thread.detach();

    return renderer->exec();
}