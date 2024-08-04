
#ifndef __CLASSATTRIBUTESYNTHETIC_H__
#define __CLASSATTRIBUTESYNTHETIC_H__

#include "JavaCodeAttribute.h"

class ConstantPool; // Forward declaration
class Stream; // Forward declaration

class ClassAttributeSynthetic : public JavaCodeAttribute
{
private:
  ClassAttributeSynthetic( const JavaString &name );

public:
  ClassAttributeSynthetic( const ClassAttributeSynthetic &other );

  virtual bool Equals( const JavaCodeAttribute &other ) const JVMX_OVERRIDE;
  bool Equals( const ClassAttributeSynthetic &other ) const JVMX_NOEXCEPT;

  virtual ~ClassAttributeSynthetic() JVMX_NOEXCEPT;

  static ClassAttributeSynthetic FromBinary( JavaString name );

  virtual DataBuffer ToBinary( const ConstantPool & ) const JVMX_OVERRIDE;
};

#endif // __CLASSATTRIBUTESYNTHETIC_H__
