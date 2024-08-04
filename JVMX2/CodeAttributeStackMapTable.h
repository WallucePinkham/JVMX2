
#ifndef __CODEATTRIBUTESTACKMAPTABLE_H__
#define __CODEATTRIBUTESTACKMAPTABLE_H__

#include "StackFrame.h"
#include "JavaCodeAttribute.h"

class Stream; // Forward declaration.

class CodeAttributeStackMapTable : public JavaCodeAttribute
{
protected:
  CodeAttributeStackMapTable( StackFrameList stackFrames );

public:
  CodeAttributeStackMapTable( CodeAttributeStackMapTable &&other );
  CodeAttributeStackMapTable( const CodeAttributeStackMapTable &other );

  virtual ~CodeAttributeStackMapTable() JVMX_NOEXCEPT;

  CodeAttributeStackMapTable &operator=(CodeAttributeStackMapTable other);

  virtual bool Equals( const JavaCodeAttribute &other ) const JVMX_OVERRIDE;
  virtual bool Equals( const CodeAttributeStackMapTable &other ) const JVMX_NOEXCEPT;

  virtual size_t GetStackFrameCount() const;
  virtual const StackFrame &GetStackFrame( size_t index ) const;

  virtual DataBuffer ToBinary( const ConstantPool & ) const JVMX_OVERRIDE;

public:
  static CodeAttributeStackMapTable FromBinary( Stream &byteStream );

protected:
  static void swap( CodeAttributeStackMapTable &left, CodeAttributeStackMapTable &right ) JVMX_NOEXCEPT;

private:
  StackFrameList m_StackFrames;
};

#endif // __CODEATTRIBUTESTACKMAPTABLE_H__
