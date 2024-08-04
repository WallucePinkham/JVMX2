
#ifndef _CODEATTRIBUTERUNTIMEVISIBLEPARAMETERANNOTATIONS__H_
#define _CODEATTRIBUTERUNTIMEVISIBLEPARAMETERANNOTATIONS__H_

#include "JavaCodeAttribute.h"
#include "ParameterAnnotationsEntry.h"

class ConstantPool; // Forward declaration
class Stream; // Forward declaration

class CodeAttributeRuntimeVisibleParameterAnnotations : public JavaCodeAttribute
{
private:
  CodeAttributeRuntimeVisibleParameterAnnotations( const JavaString &name, ParameterAnnotationsList parameterAnnotations );

public:
  CodeAttributeRuntimeVisibleParameterAnnotations( const CodeAttributeRuntimeVisibleParameterAnnotations &other );
  CodeAttributeRuntimeVisibleParameterAnnotations( CodeAttributeRuntimeVisibleParameterAnnotations &&other );

  bool Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT JVMX_OVERRIDE;
  bool Equals( const CodeAttributeRuntimeVisibleParameterAnnotations &other ) const JVMX_NOEXCEPT;

  virtual uint16_t GetNumberOfParameters() const JVMX_NOEXCEPT;
  virtual std::shared_ptr<ParameterAnnotationsEntry> GetParameterAnnotationsAt( size_t at ) const;

  virtual ~CodeAttributeRuntimeVisibleParameterAnnotations() JVMX_NOEXCEPT;

  static CodeAttributeRuntimeVisibleParameterAnnotations FromBinary( JavaString name, Stream &byteStream );

  virtual DataBuffer ToBinary( const ConstantPool & ) const JVMX_OVERRIDE;

protected:
  virtual void swap( CodeAttributeRuntimeVisibleParameterAnnotations &left, CodeAttributeRuntimeVisibleParameterAnnotations &right ) JVMX_NOEXCEPT;

protected:
  ParameterAnnotationsList m_ParameterAnnotations;
};


#endif // _CODEATTRIBUTERUNTIMEVISIBLEPARAMETERANNOTATIONS__H_
