
#include "Stream.h"
#include "InvalidArgumentException.h"
#include "IndexOutOfBoundsException.h"

#include "CodeAttributeSignature.h"

CodeAttributeSignature::CodeAttributeSignature( const CodeAttributeSignature &other ) : JavaCodeAttribute( other ), m_SignatureIndex( other.m_SignatureIndex )
{}

CodeAttributeSignature::CodeAttributeSignature( const JavaString &name, ConstantPoolIndex signatureIndex ) : JavaCodeAttribute( name, e_JavaAttributeTypeSignature ), m_SignatureIndex( signatureIndex )
{
}

CodeAttributeSignature::CodeAttributeSignature( CodeAttributeSignature &&other ) : JavaCodeAttribute( JavaString::EmptyString(), e_JavaAttributeTypeSignature )
{
  swap( *this, other );
}

CodeAttributeSignature::~CodeAttributeSignature()
{}

CodeAttributeSignature CodeAttributeSignature::FromBinary( JavaString name, Stream &byteStream )
{
  uint16_t signatureIndex = byteStream.ReadUint16();

  return CodeAttributeSignature( name, signatureIndex );
}

void CodeAttributeSignature::swap( CodeAttributeSignature &left, CodeAttributeSignature &right ) JVMX_NOEXCEPT
{
  JavaCodeAttribute::swap( left, right );
  std::swap( left.m_SignatureIndex, right.m_SignatureIndex );
}

DataBuffer CodeAttributeSignature::ToBinary( const ConstantPool & ) const
{
  DataBuffer buffer = DataBuffer::EmptyBuffer();

  return buffer.AppendUint16( static_cast<uint16_t>(m_SignatureIndex) );
}

bool CodeAttributeSignature::Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT
{
  if ( other.GetType() != m_Type )
  {
    return false;
  }

  return Equals( static_cast<const CodeAttributeSignature &>(other) );
}

bool CodeAttributeSignature::Equals( const CodeAttributeSignature &other ) const JVMX_NOEXCEPT
{
  return m_SignatureIndex == other.GetIndex();
}

ConstantPoolIndex CodeAttributeSignature::GetIndex() const JVMX_NOEXCEPT
{
  return m_SignatureIndex;
}



