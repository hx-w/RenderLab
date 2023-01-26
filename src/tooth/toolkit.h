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
#include "wkflow_context.h"

namespace ToothSpace {
	class ToothPack;

#define PY_INITENV_MODULE  "scripts.py_env_checker"
#define PY_LOADPROJ_MODULE "scripts.py_load_project"
#define PY_PALETTE_MODULE  "scripts.py_mesh_palette"

#define PY_REQUIREMENTS ("trimesh", "toml", "matplotlib")

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
	void get_tooth_pack_cache(ToothPack* /* tpack */);

	void save_tooth_pack_cache(ToothPack* /* tpack */);

	void load_meshes_to_renderer(ToothPack* /* tpack */);

	/// topological sort for nodeflow
	void topological_sort(const std::vector<LinkPair>& /* links */, std::vector<NodeId>& /* node_order */);

	void show_mesh_curvature(
		uint32_t /* id */, const std::string& /* type */, const std::string& = ""
	);

	/**
	 * Active stage for Nodes.
	 * called in workspace, invoked from GUI actions
	 */

	/// [preprocess]
	void action_node_1(std::shared_ptr<ToothPack> /* tpack */);
}


#endif
