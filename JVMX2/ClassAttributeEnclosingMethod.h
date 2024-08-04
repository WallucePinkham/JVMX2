
#ifndef __CLASSATTRIBUTEENCLOSINGMETHOD_H__
#define __CLASSATTRIBUTEENCLOSINGMETHOD_H__

#include "ExceptionTableEntry.h"
#include "JavaCodeAttribute.h"

class ConstantPool; // Forward declaration
class Stream; // Forward declaration

class ClassAttributeEnclosingMethod : public JavaCodeAttribute
{
private:
  ClassAttributeEnclosingMethod( const JavaString &name, ConstantPoolIndex classIndex, ConstantPoolIndex methodIndex );

public:
  ClassAttributeEnclosingMethod( const ClassAttributeEnclosingMethod &other );
  ClassAttributeEnclosingMethod( ClassAttributeEnclosingMethod &&other );

  virtual bool Equals( const JavaCodeAttribute &other ) const JVMX_OVERRIDE;
  bool Equals( const ClassAttributeEnclosingMethod &other ) const JVMX_NOEXCEPT;

  virtual ConstantPoolIndex GetClassIndex() const JVMX_NOEXCEPT;
  virtual ConstantPoolIndex GetMethodIndex() const JVMX_NOEXCEPT;

  virtual ~ClassAttributeEnclosingMethod() JVMX_NOEXCEPT;

  static ClassAttributeEnclosingMethod FromBinary( JavaString name, Stream &byteStream );

  virtual DataBuffer ToBinary( const ConstantPool & ) const JVMX_OVERRIDE;

protected:
  virtual void swap( ClassAttributeEnclosingMethod &left, ClassAttributeEnclosingMethod &right ) JVMX_NOEXCEPT;

protected:
  ConstantPoolIndex m_ClassIndex;
  ConstantPoolIndex m_MethodIndex;
};

#endif // __CLASSATTRIBUTEENCLOSINGMETHOD_H__

