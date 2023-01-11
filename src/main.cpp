#include <iostream>
#include <thread>
#include "render/engine.h"
#include "GUI/engine.h"
#include "tooth/engine.h"

#include <pybind11/embed.h>
#include <pybind11/numpy.h>
namespace py = pybind11;

using namespace std;

int main() {
	py::scoped_interpreter guard{};
	py::print("Python interpreter is initialized!");

	cout << "main thread ID: " << this_thread::get_id() << endl;
	auto renderer = RenderSpace::make_renderer(1200, 800);

	// thread ml_thread(train_and_test, 100, renderer->get_service());
	// ml_thread.detach();

	auto __GUI_sev = GUISpace::make_service();
	auto __tooth_sev = ToothSpace::make_service();

	return renderer->exec();
}