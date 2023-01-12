#include <iostream>
#include <thread>

#include "render/engine.h"
#include "GUI/engine.h"
#include "tooth/engine.h"


using namespace std;

int main() {
	cout << "main thread ID: " << this_thread::get_id() << endl;

	auto renderer = RenderSpace::make_renderer(1200, 800);

	auto __GUI_sev = GUISpace::make_service();

	auto __tooth_thd = thread(ToothSpace::make_service);
	__tooth_thd.detach();

	return renderer->exec();
}