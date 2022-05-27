#include <iostream>
#include <thread>
#include "render/engine.h"
#include "tooth/engine.h"
#include "tooth/execute.h"
#include "train_and_test.hpp"

using namespace std;

int main() {
    cout << "main thread ID: " << this_thread::get_id() << endl;
    auto renderer = RenderSpace::make_renderer(1200, 800);

    thread ml_thread(train_and_test, 100, renderer->get_service());
    ml_thread.detach();

    return renderer->exec();
}