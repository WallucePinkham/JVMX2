#include "GlobalConstants.h"

#include "FileSearchPathCollection.h"
#include "HelperConversion.h"

FileSearchPathCollection::FileSearchPathCollection()
{
	m_SearchPaths.insert(HelperConversion::ConvertJVMXCharToUtf8(JVMX_T("../classpath/")));
	m_SearchPaths.insert(HelperConversion::ConvertJVMXCharToUtf8(JVMX_T("classpath/")));
}

std::set<std::string> FileSearchPathCollection::GetPaths()
{
	std::lock_guard<std::recursive_mutex> lock(m_Mutex);
	return m_SearchPaths;
}

void FileSearchPathCollection::AddSearchPath(const JVMX_CHAR_TYPE* pPath)
{
	std::lock_guard<std::recursive_mutex> lock(m_Mutex);
	m_SearchPaths.insert(HelperConversion::ConvertJVMXCharToUtf8(pPath));
}