
#ifndef __JAVALONG_H__
#define __JAVALONG_H__

#include "GlobalConstants.h"
#include "IJavaVariableType.h"
#include "IConstantPoolEntryValue.h"

class JavaLong : public IJavaVariableType, public IConstantPoolEntryValue
{
private:
  explicit JavaLong( int64_t value );
  explicit JavaLong();

public:
  JavaLong( const JavaLong &other );

  virtual ~JavaLong() JVMX_NOEXCEPT;

  virtual e_JavaVariableTypes GetVariableType() const JVMX_OVERRIDE;
  virtual bool IsReferenceType() const JVMX_OVERRIDE;
  virtual bool IsIntegerCompatible() const JVMX_OVERRIDE;
  virtual bool IsNull() const JVMX_OVERRIDE;
  virtual JavaString ToString() const JVMX_OVERRIDE;

  static JavaLong FromHostInt64( int64_t hostValue );
  static JavaLong FromNetworkInt64( int64_t networkValue );
  static JavaLong FromDefault();

  virtual int64_t ToHostInt64() const;
  virtual int64_t ToNetworkInt64() const;

  virtual bool operator==(const IJavaVariableType &other) const JVMX_OVERRIDE;
  virtual bool operator==(const JavaLong &other) const;

  virtual bool operator<(const IJavaVariableType &other) const JVMX_OVERRIDE;
  virtual bool operator<(const JavaLong &other) const;

  JavaLong &operator=(const JavaLong &other);
  virtual JavaLong &operator=( const IJavaVariableType &other ) JVMX_OVERRIDE;

private:
  int64_t m_Value;
};

#endif // __JAVALONG_H__
