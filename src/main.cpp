#include <iostream>
#include <thread>
#include "render/engine.h"
#include "GUI/engine.h"

#include <pybind11/embed.h>
namespace py = pybind11;

using namespace std;

int main() {
    try {
        // py::scoped_interpreter guard{};
        // py::print("Python interpreter is initialized!");
    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    cout << "main thread ID: " << this_thread::get_id() << endl;
    auto renderer = RenderSpace::make_renderer(1200, 800);

    // thread ml_thread(train_and_test, 100, renderer->get_service());
    // ml_thread.detach();

    auto _ = GUISpace::make_service();

    return renderer->exec();
}