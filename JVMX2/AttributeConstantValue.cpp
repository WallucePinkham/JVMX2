#include "Stream.h"
#include "InvalidArgumentException.h"

#include "AttributeConstantValue.h"

AttributeConstantValue::AttributeConstantValue( JavaString name, ConstantPoolIndex constantIndex ) : JavaCodeAttribute( name, e_JavaAttributeTypeConstantValue ), m_ConstantIndex( constantIndex )
{
  if ( 0 == m_ConstantIndex )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Constant pool index of 0 is invalid." );
  }
}

AttributeConstantValue::AttributeConstantValue( const AttributeConstantValue &other ) : JavaCodeAttribute( other ), m_ConstantIndex( other.m_ConstantIndex )
{}

AttributeConstantValue::~AttributeConstantValue()
{}

AttributeConstantValue AttributeConstantValue::FromConstantPoolIndex( JavaString name, ConstantPoolIndex constantIndex )
{
  return AttributeConstantValue( name, constantIndex );
}

AttributeConstantValue AttributeConstantValue::FromBinary( JavaString name, Stream &byteStream )
{
  if ( byteStream.GetBytesLeft() < sizeof( ConstantPoolIndex ) )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Not enough bytes left in the stream." );
  }

  return AttributeConstantValue( name, static_cast<ConstantPoolIndex>(byteStream.ReadUint16()) );
}

ConstantPoolIndex AttributeConstantValue::GetValueIndex() const JVMX_NOEXCEPT
{
  return m_ConstantIndex;
}

// void AttributeConstantValue::swap( AttributeConstantValue &left, AttributeConstantValue &right ) JVMX_NOEXCEPT
// {
//   JavaCodeAttribute::swap( left, right );
//   std::swap( left.m_ConstantIndex, right.m_ConstantIndex );
// }

DataBuffer AttributeConstantValue::ToBinary( const ConstantPool & ) const
{
  return DataBuffer::EmptyBuffer().AppendUint16( static_cast<uint16_t>(m_ConstantIndex) );
}

bool AttributeConstantValue::Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT
{
  if ( other.GetType() != m_Type )
  {
    return false;
  }

  return Equals( static_cast<const AttributeConstantValue &>(other) );
}

bool AttributeConstantValue::Equals( const AttributeConstantValue &other ) const JVMX_NOEXCEPT
{
  return m_ConstantIndex == other.m_ConstantIndex;
}