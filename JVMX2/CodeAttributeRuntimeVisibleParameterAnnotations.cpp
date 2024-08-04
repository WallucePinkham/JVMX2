
#include <stdexcept>

#include "IndexOutOfBoundsException.h"
#include "CodeAttributeRuntimeVisibleParameterAnnotations.h"

CodeAttributeRuntimeVisibleParameterAnnotations::CodeAttributeRuntimeVisibleParameterAnnotations( const JavaString &name, ParameterAnnotationsList parameterAnnotations )
  : JavaCodeAttribute( name, e_JavaAttributeTypeRuntimeVisibleParameterAnnotations )
{
  m_ParameterAnnotations = parameterAnnotations;
}

CodeAttributeRuntimeVisibleParameterAnnotations::CodeAttributeRuntimeVisibleParameterAnnotations( const CodeAttributeRuntimeVisibleParameterAnnotations &other )
  : JavaCodeAttribute( other )
{
  m_ParameterAnnotations = other.m_ParameterAnnotations;
}

CodeAttributeRuntimeVisibleParameterAnnotations::CodeAttributeRuntimeVisibleParameterAnnotations( CodeAttributeRuntimeVisibleParameterAnnotations &&other )
  : JavaCodeAttribute( JavaString::EmptyString(), e_JavaAttributeTypeRuntimeVisibleParameterAnnotations )
{
  swap( *this, other );
}

bool CodeAttributeRuntimeVisibleParameterAnnotations::Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT
{
  if ( other.GetType() != m_Type )
  {
    return false;
  }

  return Equals( static_cast<const CodeAttributeRuntimeVisibleParameterAnnotations &>(other) );
}

bool CodeAttributeRuntimeVisibleParameterAnnotations::Equals( const CodeAttributeRuntimeVisibleParameterAnnotations &other ) const JVMX_NOEXCEPT
{
  return std::equal( m_ParameterAnnotations.cbegin(), m_ParameterAnnotations.cend(), other.m_ParameterAnnotations.cbegin() );
}

uint16_t CodeAttributeRuntimeVisibleParameterAnnotations::GetNumberOfParameters() const JVMX_NOEXCEPT
{
  return static_cast<uint16_t>(m_ParameterAnnotations.size());
}

std::shared_ptr<ParameterAnnotationsEntry> CodeAttributeRuntimeVisibleParameterAnnotations::GetParameterAnnotationsAt( size_t at ) const
{
  try
  {
    return m_ParameterAnnotations.at( at );
  }
  catch ( std::out_of_range & )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Index out of bounds looking for exception table index." );
  }
}

CodeAttributeRuntimeVisibleParameterAnnotations::~CodeAttributeRuntimeVisibleParameterAnnotations() JVMX_NOEXCEPT
{
}

CodeAttributeRuntimeVisibleParameterAnnotations CodeAttributeRuntimeVisibleParameterAnnotations::FromBinary( JavaString name, Stream &byteStream )
{
  ParameterAnnotationsList annotations;
  uint8_t numberOfParameters = byteStream.ReadUint8();
  //uint16_t count = byteStream.ReadUint16();

  for ( uint16_t index = 0; index < numberOfParameters; ++ index )
  {
    annotations.push_back( ParameterAnnotationsEntry::FromBinary( byteStream ) );
  }

  return CodeAttributeRuntimeVisibleParameterAnnotations( name, annotations );
}

DataBuffer CodeAttributeRuntimeVisibleParameterAnnotations::ToBinary( const ConstantPool & ) const
{
  DataBuffer buffer = DataBuffer::EmptyBuffer();

  buffer = buffer.AppendUint16( static_cast<uint16_t>(m_ParameterAnnotations.size()) );
  for ( size_t index = 0; index < m_ParameterAnnotations.size(); ++ index )
  {
    buffer = buffer.Append( m_ParameterAnnotations.at( index )->ToBinary() );
  }

  return buffer;
}

void CodeAttributeRuntimeVisibleParameterAnnotations::swap( CodeAttributeRuntimeVisibleParameterAnnotations &left, CodeAttributeRuntimeVisibleParameterAnnotations &right ) JVMX_NOEXCEPT
{
  JavaCodeAttribute::swap( left, right );
  std::swap( left.m_ParameterAnnotations, right.m_ParameterAnnotations );
}
