
#ifndef __CODEATTRIBUTESIGNATURE_H__
#define __CODEATTRIBUTESIGNATURE_H__

#include "JavaCodeAttribute.h"
#include "ExceptionTableEntry.h"

class ConstantPool; // Forward declaration
class Stream; // Forward declaration

class CodeAttributeSignature : public JavaCodeAttribute
{
private:
  CodeAttributeSignature( const JavaString &name, ConstantPoolIndex signatureIndex );

public:
  CodeAttributeSignature( const CodeAttributeSignature &other );
  CodeAttributeSignature( CodeAttributeSignature &&other );

  bool Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT JVMX_OVERRIDE;
  bool Equals( const CodeAttributeSignature &other ) const JVMX_NOEXCEPT;

  virtual ConstantPoolIndex GetIndex() const JVMX_NOEXCEPT;

  virtual ~CodeAttributeSignature() JVMX_NOEXCEPT;

  static CodeAttributeSignature FromBinary( JavaString name, Stream &byteStream );

  virtual DataBuffer ToBinary( const ConstantPool & ) const JVMX_OVERRIDE;

protected:
  virtual void swap( CodeAttributeSignature &left, CodeAttributeSignature &right ) JVMX_NOEXCEPT;

protected:
  ConstantPoolIndex m_SignatureIndex;
};

#endif // __CODEATTRIBUTESIGNATURE_H__
