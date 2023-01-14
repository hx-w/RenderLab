/*****************************************************************//**
 * \file   toolkit.h
 * \brief  common toolkits for tooth
 *		   depth generator
 *
 * \author CarOL
 * \date   January 2023
 *********************************************************************/

#ifndef TOOTH_TOOLKIT_H
#define TOOTH_TOOLKIT_H

#include <string>

#include "typedef.h"

namespace ToothSpace {

#define PY_INITENV_MODULE  "scripts.py_env_checker"
#define PY_LOADPROJ_MODULE "scripts.py_load_project"

	/// call py scripts: 'scripts/py_env_checker.py'
	bool init_workenv(std::string& /* status */);


	/// 1. check filepath(tooth group) is valid or not => return bool, log_msg
	/// 2. if valid, setup folder structure, maintain '.rdlab.toml'(config file)
	/// 
	///		create 'static/' folder if not exist
	/// 
	/// @return int:
	///		0 failed
	///		1 success
	///		2 need to be recalled by force
	int preprocess_tooth_path(
		const std::string& /* project_path */,
		bool /* force */,
		std::string& /* status */
	);


	/// load workflow params from project path (stored in config)
	void get_workflow_params(const std::string& /* project_path */, WorkflowParams&);
}


#endif
