
#ifndef _IFLOATINGPOINTBASE__H_
#define _IFLOATINGPOINTBASE__H_

#include "GlobalConstants.h"
#include "IJavaVariableType.h"

class IFloatingPointBase /*JVMX_ABSTRACT*/ : public IJavaVariableType
{
public:
  virtual ~IFloatingPointBase() {};

  virtual e_JavaVariableTypes GetVariableType() const JVMX_PURE JVMX_OVERRIDE;
  virtual bool IsReferenceType() const JVMX_PURE JVMX_OVERRIDE;
  virtual bool IsIntegerCompatible() const JVMX_PURE JVMX_OVERRIDE;
  virtual bool IsNull() const JVMX_PURE JVMX_OVERRIDE;
  virtual JavaString ToString() const JVMX_PURE JVMX_OVERRIDE;

  virtual bool operator<(const IJavaVariableType &other) const JVMX_OVERRIDE;
  virtual bool operator==(const IJavaVariableType &other) const JVMX_OVERRIDE;
  virtual bool operator!=(const IJavaVariableType &other) const JVMX_OVERRIDE;

protected:
  static JavaString ConvertDoubleToString( double value );
};

#endif // _IFLOATINGPOINTBASE__H_
