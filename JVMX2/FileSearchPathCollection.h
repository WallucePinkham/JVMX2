#pragma once

#ifndef _FILESEARCHPATHCOLLECTION__H_
#define _FILESEARCHPATHCOLLECTION__H

#include <set>
#include <mutex>

#include "IEnumerable.h"

#include <wallaroo/part.h>


class FileSearchPathCollection : public wallaroo::Part 
{
public:
	FileSearchPathCollection();

	std::set<std::string> GetPaths();
	void AddSearchPath(const JVMX_CHAR_TYPE* pPath);


private:
	std::recursive_mutex m_Mutex;
	std::set<std::string> m_SearchPaths;
};


#endif // _FILESEARCHPATHCOLLECTION__H_