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
#include <geom_types.h>


namespace ToothSpace {
	class ToothPack;

#define PY_INITENV_MODULE    "scripts.py_env_checker"
#define PY_LOADPROJ_MODULE   "scripts.py_load_project"
#define PY_PALETTE_MODULE    "scripts.py_mesh_palette"
#define PY_NURBS_MODULE      "scripts.py_nurbs_toolkit"
#define PY_TOOTHDEPTH_MODULE "scripts.py_tooth_depth"
#define PY_PARAMETER_MODULE  "scripts.py_parameter_remesh"

#define PY_REQUIREMENTS "trimesh", "toml", "matplotlib", "rtree", "numpy_indexed", "tqdm", "scipy"

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


	void compute_nurbs_curve_info(
		std::vector<geometry::Point3f>& /* points */,
		std::vector<geometry::Point3f>& /* control_points [out] */,
		std::vector<float>& /* knots [out] */
	);

	void _compute_parameter_remesh(
		uint32_t /* uns_msh [in] */,
		uint32_t& /* str_msh [out] */,
		uint32_t& /* param_msh [out]*/,
		int /* U */, 
		int /* V */
	);

	void compute_tooth_depth_GT(
		const std::vector<uint32_t>& /* meshes_id */,
		std::shared_ptr<ToothPack>,
		std::vector<float>& /* depth value for face1 [out] */
	);

	void compute_tooth_depth_ML(
		const std::vector<uint32_t>& /* meshes_id */,
		std::shared_ptr<ToothPack>,
		std::vector<float>& /* depth value for face1 [out] */
	);

	void _hover_vertex_handler(uint32_t, uint32_t);

	void _pick_vertex_handler(uint32_t, uint32_t);

	/**
	 * Active stage for Nodes.
	 * called in workspace, invoked from GUI actions
	 */

	/// [preprocess]
	void action_node_1(std::shared_ptr<ToothPack> /* tpack */, const std::string& /* heatmap style */);

	/// [pmtr_nurbs]
	void action_node_2(std::shared_ptr<ToothPack> /* tpack */);

	/// [pmtr_remesh]
	void action_node_3(std::shared_ptr<ToothPack> /* tpack */);

	/// [generator_GT]
	void action_node_4(std::shared_ptr<ToothPack> /* tpack */);
	
	/// [generator_ML]
	void action_node_5(std::shared_ptr<ToothPack> /* tpack */);
	
	/// [postprecess]
	void action_node_6(std::shared_ptr<ToothPack> /* tpack */);
}


#endif
