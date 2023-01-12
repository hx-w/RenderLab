#include "toolkit.h"

#include <iostream>
#include <pybind11/embed.h>

using namespace std;
namespace py = pybind11;



namespace ToothSpace {
	bool init_workenv(string& status) {
		try {
			py::eval_file(PY_INITENV_SCRIPT);
			status = "package loaded";
			return true;
		}
		catch (exception& e) {
			status = e.what();
			return false;
		}
	}

	bool preprocess_tooth_path(const string& path, string& status) {
		// check path is folder, and folder's elements valid
		auto _py_os = py::module_::import("os");
		if (!_py_os.attr("path").attr("isdir")(path.c_str()).cast<bool>()) {
			status = "project target must be a directory";
			return false;
		}

		return true;
	}
}
