
#ifndef __JAVADOUBLE_H__
#define __JAVADOUBLE_H__

#include "GlobalConstants.h"
#include "IFloatingPointBase.h"
#include "IConstantPoolEntryValue.h"

class JavaDouble : public IFloatingPointBase, public IConstantPoolEntryValue
{
private:
  explicit JavaDouble( double value ) JVMX_NOEXCEPT;
  explicit JavaDouble() JVMX_NOEXCEPT;

public:
  JavaDouble( const JavaDouble &other );

  virtual ~JavaDouble() JVMX_NOEXCEPT;

  virtual e_JavaVariableTypes GetVariableType() const JVMX_OVERRIDE;
  virtual bool IsReferenceType() const JVMX_OVERRIDE;
  virtual bool IsIntegerCompatible() const JVMX_OVERRIDE;
  virtual bool IsNull() const JVMX_OVERRIDE;
  virtual JavaString ToString() const JVMX_OVERRIDE;

  static JavaDouble FromNetworkValue( double networkValue );
  static JavaDouble FromHostDouble( double hostValue );
  static JavaDouble FromHostInt64( int64_t intValue );
  static JavaDouble FromDefault();

  virtual bool IsNan() const;

  virtual double ToNetworkValue() const;
  virtual double ToHostDouble() const;

  virtual bool operator==(const IJavaVariableType &other) const JVMX_OVERRIDE;
  virtual bool operator==(const JavaDouble &other) const;

  virtual bool operator<(const IJavaVariableType &other) const JVMX_OVERRIDE;
  virtual bool operator<(const JavaDouble &other) const;
  virtual bool operator>( const JavaDouble &other ) const;

  JavaDouble &operator=(const JavaDouble &other);
  virtual JavaDouble &operator=( const IJavaVariableType &other ) JVMX_OVERRIDE;

private:
  template<class T> static JavaDouble FromHostDouble( T hostValue ) JVMX_FN_DELETE;
  template<class T> static JavaDouble FromNetworkValue( T networkValue ) JVMX_FN_DELETE;
  
private:
  double m_Value;
};

#endif // __JAVADOUBLE_H__
