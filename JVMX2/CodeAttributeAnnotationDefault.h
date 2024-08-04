
#ifndef _CODEATTRIBUTEANNOTATIONDEFAULT__H_
#define _CODEATTRIBUTEANNOTATIONDEFAULT__H_

#include "JavaCodeAttribute.h"
#include "AnnotationsEntry.h"

class ConstantPool; // Forward declaration
class Stream; // Forward declaration

class CodeAttributeAnnotationDefault : public JavaCodeAttribute
{
private:
  CodeAttributeAnnotationDefault( const JavaString &name, AnnotationsEntry entry );

public:
  CodeAttributeAnnotationDefault( const CodeAttributeAnnotationDefault &other );
  CodeAttributeAnnotationDefault( CodeAttributeAnnotationDefault &&other );

  bool Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT JVMX_OVERRIDE;
  bool Equals( const CodeAttributeAnnotationDefault &other ) const JVMX_NOEXCEPT;

  uint16_t GetTypeIndex() const JVMX_NOEXCEPT;
  uint16_t GetNumberOfElementValuePairs() const JVMX_NOEXCEPT;
  const AnnotationsElementValuePair &GetElementValuePairAt( size_t index ) const;

  virtual ~CodeAttributeAnnotationDefault() JVMX_NOEXCEPT;

  static CodeAttributeAnnotationDefault FromBinary( JavaString name, Stream &byteStream );

  virtual DataBuffer ToBinary( const ConstantPool & ) const JVMX_OVERRIDE;

protected:
  virtual void swap( CodeAttributeAnnotationDefault &left, CodeAttributeAnnotationDefault &right ) JVMX_NOEXCEPT;

protected:
  AnnotationsEntry m_Entry;
};


#endif // _CODEATTRIBUTEANNOTATIONDEFAULT__H_
