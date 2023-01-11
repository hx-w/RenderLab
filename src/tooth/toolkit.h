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

namespace ToothSpace {

	/// 1. check filepath(tooth group) is valid or not => return bool, log_msg
	/// 2. if valid, setup folder structure, maintain '.rdlab.yaml'(config file)
	/// 
	///		create 'output/' folder if not exist
	bool preprocess_tooth_path(const std::string&, std::string&);

}


#endif
