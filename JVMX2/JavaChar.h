
#ifndef _JAVACHAR__H_
#define _JAVACHAR__H_

#include <string>

#include "GlobalConstants.h"

#include "IJavaVariableType.h"
#include "IConstantPoolEntryValue.h"

class JavaChar : public IJavaVariableType
{
public:
  explicit JavaChar();

  static JavaChar FromUInt16( uint16_t value );
  static JavaChar FromCChar( const char value );
  static JavaChar FromCWChar( const wchar_t value );
  static JavaChar FromChar16( const char16_t value );
  static JavaChar FromDefault();

protected:
  explicit JavaChar( char16_t value );

public:
  JavaChar( const JavaChar &other );
  explicit JavaChar( JavaChar &&other ) JVMX_NOEXCEPT;

  virtual ~JavaChar() JVMX_NOEXCEPT;

  virtual e_JavaVariableTypes GetVariableType() const JVMX_OVERRIDE;
  virtual bool IsReferenceType() const JVMX_OVERRIDE;
  virtual bool IsIntegerCompatible() const JVMX_OVERRIDE;
  virtual bool IsNull() const JVMX_OVERRIDE;
  virtual JavaString ToString() const JVMX_OVERRIDE;

  JavaChar &operator=(JavaChar other);
  virtual JavaChar &operator=(const IJavaVariableType &other ) JVMX_OVERRIDE;

  virtual bool operator==(const IJavaVariableType &other) const JVMX_OVERRIDE;
  virtual bool operator==(const JavaChar &other) const;

  virtual bool operator<(const IJavaVariableType &other) const JVMX_OVERRIDE;
  virtual bool operator<(const JavaChar &other) const;

  //char ToCChar() const;
  char16_t ToChar16() const;

  uint16_t ToUInt16() const;

public:
  static const JavaChar DefaultChar();

  // In order to conform with the general C++ standards, this method name is intentionally all lower case.
  void swap( JavaChar &left, JavaChar &right ) JVMX_NOEXCEPT;
  

private:
  char16_t m_Value;
};

#endif // _JAVACHAR__H_

