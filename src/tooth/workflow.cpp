#include "workflow.h"

#include <iostream>

#include "toolkit.h"
#include "engine.h"
#include "service.h"

using namespace std;

#define SERVICE_INST ToothEngine::get_instance()->get_service()
#define TOOLKIT_EXEC(func, prefix, ...) \
			string log_msg = ""; \
			if (!func(__VA_ARGS__ log_msg)) {	\
				SERVICE_INST->slot_add_log("error", prefix##" error, " + log_msg); \
			} else { \
				SERVICE_INST->slot_add_log("info", prefix##" successfully, " + log_msg); \
			}


namespace ToothSpace {
	Workspace::Workspace() {
		TOOLKIT_EXEC(init_workenv, "init workspace", )
	}

	void Workspace::slot_fetch_filepath(const string& filepath) {
		cout << filepath.c_str() << endl;
		TOOLKIT_EXEC(preprocess_tooth_path, "project load", filepath,)

	}

}
