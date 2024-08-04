
#ifndef _CODEATRRIBUTERUNTIMEVISIBLEANNOTATIONS__H_
#define _CODEATRRIBUTERUNTIMEVISIBLEANNOTATIONS__H_

#include "JavaCodeAttribute.h"
#include "AnnotationsEntry.h"

class ConstantPool; // Forward declaration
class Stream; // Forward declaration

class CodeAttributeRuntimeVisibleAnnotations : public JavaCodeAttribute
{
private:
  CodeAttributeRuntimeVisibleAnnotations( const JavaString &name, AnnotationsList annotations );

public:
  CodeAttributeRuntimeVisibleAnnotations( const CodeAttributeRuntimeVisibleAnnotations &other );
  CodeAttributeRuntimeVisibleAnnotations( CodeAttributeRuntimeVisibleAnnotations &&other );

  bool Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT JVMX_OVERRIDE;
  bool Equals( const CodeAttributeRuntimeVisibleAnnotations &other ) const JVMX_NOEXCEPT;

  virtual uint16_t GetNumberOfAnnotations() const JVMX_NOEXCEPT;
  virtual std::shared_ptr<AnnotationsEntry> GetAnnotationAt( size_t at ) const;

  virtual ~CodeAttributeRuntimeVisibleAnnotations() JVMX_NOEXCEPT;

  static CodeAttributeRuntimeVisibleAnnotations FromBinary( JavaString name, Stream &byteStream );

  virtual DataBuffer ToBinary( const ConstantPool & ) const JVMX_OVERRIDE;

protected:
  virtual void swap( CodeAttributeRuntimeVisibleAnnotations &left, CodeAttributeRuntimeVisibleAnnotations &right ) JVMX_NOEXCEPT;

protected:
  AnnotationsList m_Annotations;
};

#endif // _CODEATRRIBUTERUNTIMEVISIBLEANNOTATIONS__H_

