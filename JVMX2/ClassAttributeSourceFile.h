
#ifndef __CLASSATTRIBUTESOURCEFILE_H__
#define __CLASSATTRIBUTESOURCEFILE_H__

#include "JavaCodeAttribute.h"

class ConstantPool; // Forward declaration
class Stream; // Forward declaration

class ClassAttributeSourceFile : public JavaCodeAttribute
{
private:
  ClassAttributeSourceFile( const JavaString &name, ConstantPoolIndex sourceFileIndex );

public:
  ClassAttributeSourceFile( const ClassAttributeSourceFile &other );
  ClassAttributeSourceFile( ClassAttributeSourceFile &&other );

  bool Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT JVMX_OVERRIDE;
  bool Equals( const ClassAttributeSourceFile &other ) const JVMX_NOEXCEPT;

  virtual ConstantPoolIndex GetIndex() const JVMX_NOEXCEPT;

  virtual ~ClassAttributeSourceFile() JVMX_NOEXCEPT;

  static ClassAttributeSourceFile FromBinary( JavaString name, Stream &byteStream );

  virtual DataBuffer ToBinary( const ConstantPool & ) const JVMX_OVERRIDE;

protected:
  virtual void swap( ClassAttributeSourceFile &left, ClassAttributeSourceFile &right ) JVMX_NOEXCEPT;

protected:
  ConstantPoolIndex m_SourceFileIndex;
};

#endif // __CLASSATTRIBUTESOURCEFILE_H__
