#include "workflow.h"

#include <iostream>

#include "toolkit.h"
#include "engine.h"
#include "service.h"

using namespace std;

#define SERVICE_INST ToothEngine::get_instance()->get_service()

namespace ToothSpace {
	void Workspace::slot_fetch_filepath(const string& filepath) {
		cout << filepath.c_str() << endl;
		string log_msg = "";
		if (!preprocess_tooth_path(filepath, log_msg)) {
			SERVICE_INST->slot_add_log("error", "project load error, " + log_msg);
			return;
		}


		SERVICE_INST->slot_add_log("info", "project load successfully, " + filepath);
	}
}
