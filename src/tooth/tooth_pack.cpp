#include "tooth_pack.h"
#include "toolkit.h"

using namespace std;
using namespace ToothSpace;


ToothPack::ToothPack(int wkflow_id, const string& filepath) {
	auto _p1 = filepath.find_last_of('/'); // in case of no-windows os
	auto _p2 = filepath.find_last_of('\\');
	_p1 = (_p1 == string::npos ? 0 : _p1 + 1);
	_p2 = (_p2 == string::npos ? 0 : _p2 + 1);
	auto wkflow_name = filepath.substr(max(_p1, _p2));

	wkflow_ctx = make_shared<WorkflowContext>(
		WorkflowContext(wkflow_id, wkflow_name)
	);

	// cached in config
	get_tooth_pack_cache(filepath, this);

	basedir = filepath;
}
