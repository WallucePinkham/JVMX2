
#ifndef _CODEATTRIBUTELINENUMBERTABLE__H_
#define _CODEATTRIBUTELINENUMBERTABLE__H_

#include "JavaCodeAttribute.h"
#include "LineNumberTableEntry.h"

class ConstantPool; // Forward declaration
class Stream; // Forward declaration

class CodeAttributeLineNumberTable : public JavaCodeAttribute
{
private:
  CodeAttributeLineNumberTable( const JavaString &name, LineNumberTable lineNumbers );

public:
  CodeAttributeLineNumberTable( const CodeAttributeLineNumberTable &other );
  CodeAttributeLineNumberTable( CodeAttributeLineNumberTable &&other );

  bool Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT JVMX_OVERRIDE;
  bool Equals( const CodeAttributeLineNumberTable &other ) const JVMX_NOEXCEPT;

  virtual uint16_t GetNumberOfLineNumbers() const;
  virtual uint16_t GetLineNumberAt( size_t at ) const;
  virtual uint16_t GetStartPositionAt( size_t at ) const;

  virtual ~CodeAttributeLineNumberTable() JVMX_NOEXCEPT;

  static CodeAttributeLineNumberTable FromBinary( JavaString name, Stream &byteStream );

  virtual DataBuffer ToBinary( const ConstantPool & ) const JVMX_OVERRIDE;

protected:
  virtual void swap( CodeAttributeLineNumberTable &left, CodeAttributeLineNumberTable &right ) JVMX_NOEXCEPT;

protected:
  LineNumberTable m_LineNumbers;
};

#endif // _CODEATTRIBUTELINENUMBERTABLE__H_
