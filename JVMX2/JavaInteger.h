#ifndef __JAVAINTEGER_H__
#define __JAVAINTEGER_H__

#include <memory>

#include "GlobalConstants.h"
#include "IJavaVariableType.h"
#include "IConstantPoolEntryValue.h"

class JavaChar;
class JavaBool;
class JavaByte;
class JavaShort;

class JavaInteger : public IJavaVariableType, public IConstantPoolEntryValue
{
private:
  explicit JavaInteger( int32_t value );
  explicit JavaInteger();

public:
  JavaInteger( const JavaInteger &other );

  virtual ~JavaInteger() JVMX_NOEXCEPT;

  virtual e_JavaVariableTypes GetVariableType() const JVMX_OVERRIDE;
  virtual bool IsReferenceType() const JVMX_OVERRIDE;
  virtual bool IsIntegerCompatible() const JVMX_OVERRIDE;
  virtual bool IsNull() const JVMX_OVERRIDE;
  virtual JavaString ToString() const JVMX_OVERRIDE;

  static JavaInteger FromHostInt32( int32_t hostValue );
  static JavaInteger FromNetworkInt32( const int32_t networkValue );
  static JavaInteger FromDefault();

  static JavaInteger FromChar( const JavaChar &value );
  static JavaInteger FromBool( const JavaBool &value );
  static JavaInteger FromByte( const JavaByte &value );
  static JavaInteger FromShort( const JavaShort &value );

  virtual int32_t ToHostInt32() const;
  virtual int32_t ToNetworkInt32() const;

  virtual bool operator==( const IJavaVariableType &other ) const JVMX_OVERRIDE;
  virtual bool operator==( const JavaInteger &other ) const;

  virtual bool operator<( const IJavaVariableType &other ) const JVMX_OVERRIDE;
  virtual bool operator<( const JavaInteger &other ) const;

  virtual boost::intrusive_ptr<JavaChar> ToChar() const;
  virtual boost::intrusive_ptr<JavaByte> ToByte() const;

  virtual boost::intrusive_ptr<JavaBool> ToBool() const;
  virtual boost::intrusive_ptr<JavaShort> ToShort() const;

  JavaInteger &operator=( const JavaInteger &other );
  JavaInteger &operator=( const IJavaVariableType &other );

private:
  int32_t m_Value;
};

#endif // __JAVAINTEGER_H__
