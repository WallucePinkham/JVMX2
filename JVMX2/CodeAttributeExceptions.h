
#ifndef __CODEATTRIBUTEEXCEPTIONS_H__
#define __CODEATTRIBUTEEXCEPTIONS_H__

#include "JavaCodeAttribute.h"
#include "ExceptionTableEntry.h"

class ConstantPool; // Forward declaration
class Stream; // Forward declaration

class CodeAttributeExceptions : public JavaCodeAttribute
{
private:
  CodeAttributeExceptions( const JavaString &name, ExceptionTableIndexList list );

public:
  CodeAttributeExceptions( const CodeAttributeExceptions &other );
  CodeAttributeExceptions( CodeAttributeExceptions &&other );

  bool Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT JVMX_OVERRIDE;
  bool Equals( const CodeAttributeExceptions &other ) const JVMX_NOEXCEPT;

  virtual uint16_t GetNumberOfExceptions() const;
  virtual uint16_t GetIndexAt( size_t at ) const;

  virtual ~CodeAttributeExceptions() JVMX_NOEXCEPT;

  static CodeAttributeExceptions FromBinary( JavaString name, Stream &byteStream );

  virtual DataBuffer ToBinary( const ConstantPool & ) const JVMX_OVERRIDE;

protected:
  virtual void swap( CodeAttributeExceptions &left, CodeAttributeExceptions &right ) JVMX_NOEXCEPT;

protected:
  ExceptionTableIndexList m_ExceptionIndices;
};

#endif // __CODEATTRIBUTEEXCEPTIONS_H__
