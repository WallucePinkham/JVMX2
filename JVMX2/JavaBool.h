
#ifndef _JAVABOOL__H_
#define _JAVABOOL__H_

#include "GlobalConstants.h"
#include "IJavaVariableType.h"
#include "IConstantPoolEntryValue.h"

class JavaBool : public IJavaVariableType
{
private:
  explicit JavaBool( bool value );
  JavaBool();

public:
  JavaBool( const JavaBool &other );

  virtual ~JavaBool() JVMX_NOEXCEPT{};

  virtual e_JavaVariableTypes GetVariableType() const JVMX_OVERRIDE;
  virtual bool IsReferenceType() const JVMX_OVERRIDE;
  virtual bool IsIntegerCompatible() const JVMX_OVERRIDE;
  virtual bool IsNull() const JVMX_OVERRIDE;
  virtual JavaString ToString() const JVMX_OVERRIDE;

  static JavaBool FromBool( bool value );
  static JavaBool FromUint16( int16_t value );
  static JavaBool FromDefault();

  virtual bool ToBool() const;
  virtual int16_t ToUint16() const;

  virtual bool operator==(const IJavaVariableType &other) const JVMX_OVERRIDE;
  virtual bool operator==(const JavaBool &other) const;

  virtual bool operator<(const IJavaVariableType &other) const JVMX_OVERRIDE;
  virtual bool operator<(const JavaBool &other) const;

  virtual JavaBool &operator=( const IJavaVariableType &other ) JVMX_OVERRIDE;

private:
  bool m_Value;
};

#endif // _JAVABOOL__H_
