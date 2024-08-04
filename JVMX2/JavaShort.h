
#ifndef _JAVASHORT__H_
#define _JAVASHORT__H_

#include "GlobalConstants.h"
#include "IJavaVariableType.h"
#include "IConstantPoolEntryValue.h"

class JavaShort : public IJavaVariableType
{
private:
  explicit JavaShort( int16_t value );
  explicit JavaShort();

public:
  JavaShort( const JavaShort &other );

  virtual ~JavaShort() JVMX_NOEXCEPT JVMX_OVERRIDE;

  virtual e_JavaVariableTypes GetVariableType() const JVMX_OVERRIDE;
  virtual bool IsReferenceType() const JVMX_OVERRIDE;
  virtual bool IsIntegerCompatible() const JVMX_OVERRIDE;
  virtual bool IsNull() const JVMX_OVERRIDE;
  virtual JavaString ToString() const JVMX_OVERRIDE;

  static JavaShort FromHostInt16( int16_t hostValue );
  static JavaShort FromNetworkInt16( const int16_t networkValue );
  static JavaShort FromDefault();

  virtual int16_t ToHostInt16() const;
  virtual int16_t ToNetworkInt16() const;

  virtual bool operator==(const IJavaVariableType &other) const JVMX_OVERRIDE;
  virtual bool operator==(const JavaShort &other) const;

  virtual bool operator<(const IJavaVariableType &other) const JVMX_OVERRIDE;
  virtual bool operator<(const JavaShort &other) const;

  virtual JavaShort &operator=( const IJavaVariableType &other ) JVMX_OVERRIDE;

private:
  int16_t m_Value;
};

#endif // _JAVASHORT__H_
