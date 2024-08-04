
#ifndef _CODEATRRIBUTERUNTIMEINVISIBLEANNOTATIONS__H_
#define _CODEATRRIBUTERUNTIMEINVISIBLEANNOTATIONS__H_

#include "JavaCodeAttribute.h"
#include "AnnotationsEntry.h"

class ConstantPool; // Forward declaration
class Stream; // Forward declaration

class CodeAttributeRuntimeInvisibleAnnotations : public JavaCodeAttribute
{
private:
  CodeAttributeRuntimeInvisibleAnnotations( const JavaString &name, AnnotationsList annotations );

public:
  CodeAttributeRuntimeInvisibleAnnotations( const CodeAttributeRuntimeInvisibleAnnotations &other );
  CodeAttributeRuntimeInvisibleAnnotations( CodeAttributeRuntimeInvisibleAnnotations &&other );

  bool Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT JVMX_OVERRIDE;
  bool Equals( const CodeAttributeRuntimeInvisibleAnnotations &other ) const JVMX_NOEXCEPT;

  virtual uint16_t GetNumberOfAnnotations() const JVMX_NOEXCEPT;
  virtual std::shared_ptr<AnnotationsEntry> GetAnnotationAt( size_t at ) const;

  virtual ~CodeAttributeRuntimeInvisibleAnnotations() JVMX_NOEXCEPT;

  static CodeAttributeRuntimeInvisibleAnnotations FromBinary( JavaString name, Stream &byteStream );

  virtual DataBuffer ToBinary( const ConstantPool & ) const JVMX_OVERRIDE;

protected:
  virtual void swap( CodeAttributeRuntimeInvisibleAnnotations &left, CodeAttributeRuntimeInvisibleAnnotations &right ) JVMX_NOEXCEPT;

protected:
  AnnotationsList m_Annotations;
};

#endif // _CODEATRRIBUTERUNTIMEINVISIBLEANNOTATIONS__H_

