
#ifndef __CODEATTRIBUTEUNKOWN_H__
#define __CODEATTRIBUTEUNKOWN_H__

#include "JavaCodeAttribute.h"
#include "JavaAttributeUnknown.h"

class Stream; // Forward declaration

class CodeAttributeUnknown : public JavaCodeAttribute, public JavaAttributeUnknown
{
private:
  CodeAttributeUnknown( const JavaString &name, Stream &byteStream, size_t byteLength );

public:
  CodeAttributeUnknown( const CodeAttributeUnknown &other );
  CodeAttributeUnknown( CodeAttributeUnknown &&other );

  CodeAttributeUnknown &operator=(CodeAttributeUnknown other);

  bool Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT JVMX_OVERRIDE;
  bool Equals( const CodeAttributeUnknown &other ) const JVMX_NOEXCEPT;

  virtual ~CodeAttributeUnknown() JVMX_NOEXCEPT;

  static CodeAttributeUnknown FromBinary( JavaString name, Stream &stream, size_t byteCount );


  virtual DataBuffer ToBinary( const ConstantPool & ) const JVMX_OVERRIDE;

protected:
  virtual void swap( CodeAttributeUnknown &left, CodeAttributeUnknown &right ) JVMX_NOEXCEPT;
};



#endif // __CODEATTRIBUTEUNKOWN_H__
