#include "toolkit.h"

#include <iostream>
#include <vector>
#include <pybind11/embed.h>

using namespace std;
namespace py = pybind11;


namespace ToothSpace {
	bool init_workenv(string& status) {
		try {
			// init interpreter for current thread
			py::scoped_interpreter guard{};
			auto _py_pkg = py::module_::import(PY_INITENV_MODULE);
			auto reqs = py::make_tuple(PY_REQUIREMENTS);
			_py_pkg.attr("make_requirements_installed")(
				reqs, "https://pypi.tuna.tsinghua.edu.cn/simple"
			);
			status = "package loaded";
			return true;
		}
		catch (exception& e) {
			status = e.what();
			return false;
		}
	}

	int preprocess_tooth_path(const string& path, bool force, string& status) {
		py::scoped_interpreter guard{};
		// check path is folder, and folder's elements valid
		auto _py_os = py::module_::import("os");
		if (!_py_os.attr("path").attr("isdir")(path).cast<bool>()) {
			status = "project target must be a directory";
			return 0;
		}

		// work with py script
		auto _py_pkg = py::module_::import(PY_LOADPROJ_MODULE);
		auto target_files = _py_pkg.attr("get_project_files")(path);
		if (target_files.cast<py::list>().empty()) {
			status = "not a valid project";
			return 0;
		}
		if (!_py_pkg.attr("update_config")(path, target_files, force).cast<bool>()) {
			status = "project source file changed";
			return 2;
		}
		else {
			status = "config updated";
			return 1;
		}
	}

	void get_workflow_params(const string& path, WkflowCtxPtr wkflow_ctx) {
		/// [TODO] compelete wkflow_ctx->node_states and node_order
	}
}
