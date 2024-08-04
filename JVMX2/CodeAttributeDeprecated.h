
#ifndef _CODEATTRIBUTEDEPRECATED__H_
#define _CODEATTRIBUTEDEPRECATED__H_

#include "JavaCodeAttribute.h"

class ConstantPool; // Forward declaration
class Stream; // Forward declaration

class CodeAttributeDeprecated : public JavaCodeAttribute
{
private:
  CodeAttributeDeprecated( const JavaString &name );

public:
  CodeAttributeDeprecated( const CodeAttributeDeprecated &other );
  CodeAttributeDeprecated( CodeAttributeDeprecated &&other );

  bool Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT JVMX_OVERRIDE;
  bool Equals( const CodeAttributeDeprecated &other ) const JVMX_NOEXCEPT;

  virtual ~CodeAttributeDeprecated() JVMX_NOEXCEPT;

  static CodeAttributeDeprecated FromBinary( JavaString name );

  virtual DataBuffer ToBinary( const ConstantPool & ) const JVMX_OVERRIDE;

protected:
  virtual void swap( CodeAttributeDeprecated &left, CodeAttributeDeprecated &right ) JVMX_NOEXCEPT;
};

#endif // _CODEATTRIBUTEDEPRECATED__H_
