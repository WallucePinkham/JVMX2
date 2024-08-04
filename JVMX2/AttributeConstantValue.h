
#ifndef __ATTRIBUTECONSTANTVALUE_H__
#define __ATTRIBUTECONSTANTVALUE_H__

#include "JavaCodeAttribute.h"

class Stream; // Forward declaration

class AttributeConstantValue : public JavaCodeAttribute
{
  AttributeConstantValue( JavaString name, ConstantPoolIndex constantIndex );

public:
  AttributeConstantValue( const AttributeConstantValue &other );

  virtual ~AttributeConstantValue() JVMX_NOEXCEPT;

  static AttributeConstantValue FromConstantPoolIndex( JavaString name, ConstantPoolIndex constantIndex );
  static AttributeConstantValue FromBinary( JavaString name, Stream &byteStream );

  bool Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT JVMX_OVERRIDE;
  bool Equals( const AttributeConstantValue &other ) const JVMX_NOEXCEPT;

  virtual DataBuffer ToBinary( const ConstantPool & ) const JVMX_OVERRIDE;

  virtual ConstantPoolIndex GetValueIndex() const JVMX_NOEXCEPT;

  //virtual void swap( AttributeConstantValue &left, AttributeConstantValue &right ) JVMX_NOEXCEPT;

protected:
  ConstantPoolIndex m_ConstantIndex;
};

#endif // __ATTRIBUTECONSTANTVALUE_H__
