#pragma once

#ifndef _IFLOATINGPOINTBASE__H_
#define _IFLOATINGPOINTBASE__H_

#include "GlobalConstants.h"
#include "IJavaVariableType.h"

class IFloatingPointBase : public IJavaVariableType
{
public:
  virtual ~IFloatingPointBase() {};

  virtual e_JavaVariableTypes GetVariableType() const JVMX_OVERRIDE JVMX_PURE;
  virtual bool IsReferenceType() const JVMX_OVERRIDE JVMX_PURE;
  virtual bool IsIntegerCompatible() const JVMX_OVERRIDE JVMX_PURE;
  virtual bool IsNull() const JVMX_OVERRIDE JVMX_PURE;
  virtual JavaString ToString() const JVMX_OVERRIDE JVMX_PURE;

  virtual bool operator<(const IJavaVariableType &other) const JVMX_OVERRIDE;
  virtual bool operator==(const IJavaVariableType &other) const JVMX_OVERRIDE;
  virtual bool operator!=(const IJavaVariableType &other) const JVMX_OVERRIDE;

protected:
  static JavaString ConvertDoubleToString( double value );
};

#endif // _IFLOATINGPOINTBASE__H_
