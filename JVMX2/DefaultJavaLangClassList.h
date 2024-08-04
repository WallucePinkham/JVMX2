
#ifndef _DEFAULTJAVALANGCLASSLIST__H_
#define _DEFAULTJAVALANGCLASSLIST__H_

#include <map>

#include "IJavaLangClassList.h"

class DefaultJavaLangClassList : public IJavaLangClassList
{
public:
  virtual ~DefaultJavaLangClassList() {};

  virtual void Add( const JavaString &typeName, boost::intrusive_ptr<ObjectReference> entry ) JVMX_OVERRIDE;
  virtual boost::intrusive_ptr<ObjectReference> Find( const JavaString &typeName ) const JVMX_OVERRIDE;

  virtual size_t GetCount() const JVMX_OVERRIDE;
  virtual boost::intrusive_ptr<ObjectReference> GetByIndex(size_t index) JVMX_OVERRIDE;

private:
  std::vector<std::pair<JavaString, boost::intrusive_ptr<ObjectReference>>> m_Entries;
};

#endif // _DEFAULTJAVALANGCLASSLIST__H_
