
#ifndef _CODEATTRIBUTERUNTIMEINVISIBLEPARAMETERANNOTATIONS__H_
#define _CODEATTRIBUTERUNTIMEINVISIBLEPARAMETERANNOTATIONS__H_

#include "JavaCodeAttribute.h"
#include "ParameterAnnotationsEntry.h"

class ConstantPool; // Forward declaration
class Stream; // Forward declaration

class CodeAttributeRuntimeInvisibleParameterAnnotations : public JavaCodeAttribute
{
private:
  CodeAttributeRuntimeInvisibleParameterAnnotations( const JavaString &name, ParameterAnnotationsList parameterAnnotations );

public:
  CodeAttributeRuntimeInvisibleParameterAnnotations( const CodeAttributeRuntimeInvisibleParameterAnnotations &other );
  CodeAttributeRuntimeInvisibleParameterAnnotations( CodeAttributeRuntimeInvisibleParameterAnnotations &&other );

  bool Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT JVMX_OVERRIDE;
  bool Equals( const CodeAttributeRuntimeInvisibleParameterAnnotations &other ) const JVMX_NOEXCEPT;

  virtual uint16_t GetNumberOfParameters() const JVMX_NOEXCEPT;
  virtual std::shared_ptr<ParameterAnnotationsEntry> GetParameterAnnotationsAt( size_t at ) const;

  virtual ~CodeAttributeRuntimeInvisibleParameterAnnotations() JVMX_NOEXCEPT;

  static CodeAttributeRuntimeInvisibleParameterAnnotations FromBinary( JavaString name, Stream &byteStream );

  virtual DataBuffer ToBinary( const ConstantPool & ) const JVMX_OVERRIDE;

protected:
  virtual void swap( CodeAttributeRuntimeInvisibleParameterAnnotations &left, CodeAttributeRuntimeInvisibleParameterAnnotations &right ) JVMX_NOEXCEPT;

protected:
  ParameterAnnotationsList m_ParameterAnnotations;
};

#endif // _CODEATTRIBUTERUNTIMEINVISIBLEPARAMETERANNOTATIONS__H_
