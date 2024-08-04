
#ifndef _JAVABYTE__H_
#define _JAVABYTE__H_

#include "GlobalConstants.h"
#include "IJavaVariableType.h"
#include "IConstantPoolEntryValue.h"

class JavaByte : public IJavaVariableType
{
private:
  explicit JavaByte( int8_t value );
  explicit JavaByte();

public:
  JavaByte( const JavaByte &other );

  virtual ~JavaByte() JVMX_NOEXCEPT;

  virtual e_JavaVariableTypes GetVariableType() const JVMX_OVERRIDE;
  virtual bool IsReferenceType() const JVMX_OVERRIDE;
  virtual bool IsIntegerCompatible() const JVMX_OVERRIDE;
  virtual bool IsNull() const JVMX_OVERRIDE;
  virtual JavaString ToString() const JVMX_OVERRIDE;

  static JavaByte FromHostInt8( int8_t hostValue );
  static JavaByte FromNetworkInt8( const int8_t networkValue );
  static JavaByte FromDefault();

  virtual int8_t ToHostInt8() const;
  virtual int8_t ToNetworkInt8() const;

  virtual bool operator==(const IJavaVariableType &other) const JVMX_OVERRIDE;
  virtual bool operator==(const JavaByte &other) const;

  virtual bool operator<(const IJavaVariableType &other) const JVMX_OVERRIDE;
  virtual bool operator<(const JavaByte &other) const;

  virtual JavaByte &operator=( const IJavaVariableType &other ) JVMX_OVERRIDE;

private:
  int8_t m_Value;
};

#endif // _JAVABYTE__H_
