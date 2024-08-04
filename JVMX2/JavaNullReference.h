/*

#ifndef _JAVANULLREFERENCE__H_
#define _JAVANULLREFERENCE__H_

#include "GlobalConstants.h"
#include "IJavaVariableType.h"

class JavaNullReference : public IJavaVariableType
{
public:
  JavaNullReference() JVMX_FN_DELETE;
  virtual ~JavaNullReference() JVMX_NOEXCEPT{};

  virtual bool operator==(const IJavaVariableType &other) const JVMX_OVERRIDE;
  virtual bool operator==(const JavaNullReference &);
  virtual bool operator<(const IJavaVariableType &) const JVMX_OVERRIDE;
  virtual bool operator<(const JavaNullReference &) const;

  virtual e_JavaVariableTypes GetVariableType() const JVMX_OVERRIDE;
  virtual bool IsReferenceType() const JVMX_OVERRIDE;
  virtual bool IsIntegerCompatible() const JVMX_OVERRIDE;
  virtual bool IsNull() const JVMX_OVERRIDE;
  virtual JavaString ToString() const JVMX_OVERRIDE;
};

#endif // _JAVANULLREFERENCE__H_
*/
