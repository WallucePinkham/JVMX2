
#ifndef _CLASSATTRIBUTESOURCEDEBUGEXTENSION__H_
#define _CLASSATTRIBUTESOURCEDEBUGEXTENSION__H_

#include "JavaCodeAttribute.h"

class ConstantPool; // Forward declaration
class Stream; // Forward declaration

class ClassAttributeSourceDebugExtension : public JavaCodeAttribute
{
private:
  ClassAttributeSourceDebugExtension( const JavaString &name, const DataBuffer &debugExtension );

public:
  ClassAttributeSourceDebugExtension( const ClassAttributeSourceDebugExtension &other );
  ClassAttributeSourceDebugExtension( ClassAttributeSourceDebugExtension &&other );

  bool Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT JVMX_OVERRIDE;
  bool Equals( const ClassAttributeSourceDebugExtension &other ) const JVMX_NOEXCEPT;

  virtual const DataBuffer &GetDebugExtension() const JVMX_NOEXCEPT;

  virtual ~ClassAttributeSourceDebugExtension() JVMX_NOEXCEPT;

  static ClassAttributeSourceDebugExtension FromBinary( JavaString name, Stream &buffer );

  virtual DataBuffer ToBinary( const ConstantPool & ) const JVMX_OVERRIDE;

protected:
  virtual void swap( ClassAttributeSourceDebugExtension &left, ClassAttributeSourceDebugExtension &right ) JVMX_NOEXCEPT;

protected:
  DataBuffer m_DebugExtension;
};

#endif // _CLASSATTRIBUTESOURCEDEBUGEXTENSION__H_
