
#ifndef _IJAVALANGCLASSLIST__H_
#define _IJAVALANGCLASSLIST__H_

#include <wallaroo/part.h>

#include "JavaTypes.h"

class IJavaLangClassList /*JVMX_ABSTRACT*/ : public wallaroo::Part
{
public:
  virtual ~IJavaLangClassList() {};

  virtual void Add( const JavaString &typeName, boost::intrusive_ptr<ObjectReference> entry ) JVMX_PURE;
  virtual boost::intrusive_ptr<ObjectReference> Find( const JavaString &typeName ) const JVMX_PURE;
  virtual size_t GetCount() const JVMX_PURE;
  virtual boost::intrusive_ptr<ObjectReference> GetByIndex(size_t index) JVMX_PURE;

protected:
  IJavaLangClassList() {};
};

#endif // _IJAVALANGCLASSLIST__H_
