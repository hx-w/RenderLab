#include <iostream>
#include <thread>

#include "render/engine.h"
#include "GUI/engine.h"
#include "tooth/engine.h"

using namespace std;

int main() {
	clog << "main thread ID: " << this_thread::get_id() << endl;

	GUISpace::make_service();

	thread(ToothSpace::make_service).detach();

	return RenderSpace::make_renderer(1600, 900)->exec();
}
