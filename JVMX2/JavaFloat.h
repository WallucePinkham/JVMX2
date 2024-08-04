
#ifndef __JAVAFLOAT_H__
#define __JAVAFLOAT_H__

#include "GlobalConstants.h"
#include "IFloatingPointBase.h"
#include "IConstantPoolEntryValue.h"

class JavaFloat : public IFloatingPointBase, public IConstantPoolEntryValue
{
private:
  explicit JavaFloat( float value ) JVMX_NOEXCEPT;
  explicit JavaFloat() JVMX_NOEXCEPT;

public:
  JavaFloat( const JavaFloat &other );

  JavaFloat &operator=( const JavaFloat &other );


  virtual ~JavaFloat() JVMX_NOEXCEPT;

  virtual e_JavaVariableTypes GetVariableType() const JVMX_OVERRIDE;
  virtual bool IsReferenceType() const JVMX_OVERRIDE;
  virtual bool IsIntegerCompatible() const JVMX_OVERRIDE;
  virtual bool IsNull() const JVMX_OVERRIDE;
  virtual JavaString ToString() const JVMX_OVERRIDE;

  virtual bool IsNan() const;

  static JavaFloat FromNetworkValue( float hostValue );
  static JavaFloat FromHostFloat( float hostValue );
  static JavaFloat FromHostFloat( double hostValue ) JVMX_FN_DELETE;
  static JavaFloat FromHostDouble( double hostValue );
  static JavaFloat FromHostDouble( float hostValue ) JVMX_FN_DELETE;
  static JavaFloat FromDefault();

  virtual float ToNetworkValue() const;
  virtual float ToHostFloat() const;

  virtual bool operator==(const IJavaVariableType &other) const JVMX_OVERRIDE;
  virtual bool operator==(const JavaFloat &other) const;

  virtual bool operator<(const IJavaVariableType &other) const JVMX_OVERRIDE;
  virtual bool operator<(const JavaFloat &other) const;
  virtual bool operator>(const JavaFloat &other) const;

  virtual JavaFloat &operator=( const IJavaVariableType &other ) JVMX_OVERRIDE;

private:
  template<class T> static JavaFloat FromHostFloat( T hostValue );
  template<class T> static JavaFloat FromNetworkValue( T hostValue );

private:
  float m_Value;
};

#endif // __JAVAFLOAT_H__
