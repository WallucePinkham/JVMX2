
#include <stdexcept>

#include "IndexOutOfBoundsException.h"
#include "CodeAttributeRuntimeInvisibleParameterAnnotations.h"

CodeAttributeRuntimeInvisibleParameterAnnotations::CodeAttributeRuntimeInvisibleParameterAnnotations( const JavaString &name, ParameterAnnotationsList parameterAnnotations )
  : JavaCodeAttribute( name, e_JavaAttributeTypeRuntimeInvisibleParameterAnnotations )
{
  m_ParameterAnnotations = parameterAnnotations;
}

CodeAttributeRuntimeInvisibleParameterAnnotations::CodeAttributeRuntimeInvisibleParameterAnnotations( const CodeAttributeRuntimeInvisibleParameterAnnotations &other )
  : JavaCodeAttribute( other )
{
  m_ParameterAnnotations = other.m_ParameterAnnotations;
}

CodeAttributeRuntimeInvisibleParameterAnnotations::CodeAttributeRuntimeInvisibleParameterAnnotations( CodeAttributeRuntimeInvisibleParameterAnnotations &&other )
  : JavaCodeAttribute( JavaString::EmptyString(), e_JavaAttributeTypeRuntimeInvisibleParameterAnnotations )
{
  swap( *this, other );
}

bool CodeAttributeRuntimeInvisibleParameterAnnotations::Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT
{
  if ( other.GetType() != m_Type )
  {
    return false;
  }

  return Equals( static_cast<const CodeAttributeRuntimeInvisibleParameterAnnotations &>(other) );
}

bool CodeAttributeRuntimeInvisibleParameterAnnotations::Equals( const CodeAttributeRuntimeInvisibleParameterAnnotations &other ) const JVMX_NOEXCEPT
{
  return std::equal( m_ParameterAnnotations.cbegin(), m_ParameterAnnotations.cend(), other.m_ParameterAnnotations.cbegin() );
}

uint16_t CodeAttributeRuntimeInvisibleParameterAnnotations::GetNumberOfParameters() const JVMX_NOEXCEPT
{
  return static_cast<uint16_t>(m_ParameterAnnotations.size());
}

std::shared_ptr<ParameterAnnotationsEntry> CodeAttributeRuntimeInvisibleParameterAnnotations::GetParameterAnnotationsAt( size_t at ) const
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

CodeAttributeRuntimeInvisibleParameterAnnotations::~CodeAttributeRuntimeInvisibleParameterAnnotations() JVMX_NOEXCEPT
{
}

CodeAttributeRuntimeInvisibleParameterAnnotations CodeAttributeRuntimeInvisibleParameterAnnotations::FromBinary( JavaString name, Stream &byteStream )
{
  ParameterAnnotationsList annotations;
  uint16_t count = byteStream.ReadUint16();

  for ( uint16_t index = 0; index < count; ++ index )
  {
    annotations.push_back( ParameterAnnotationsEntry::FromBinary( byteStream ) );
  }

  return CodeAttributeRuntimeInvisibleParameterAnnotations( name, annotations );
}

DataBuffer CodeAttributeRuntimeInvisibleParameterAnnotations::ToBinary( const ConstantPool & ) const
{
  DataBuffer buffer = DataBuffer::EmptyBuffer();

  buffer = buffer.AppendUint16( static_cast<uint16_t>(m_ParameterAnnotations.size()) );
  for ( size_t index = 0; index < m_ParameterAnnotations.size(); ++ index )
  {
    buffer = buffer.Append( m_ParameterAnnotations.at( index )->ToBinary() );
  }

  return buffer;
}

void CodeAttributeRuntimeInvisibleParameterAnnotations::swap( CodeAttributeRuntimeInvisibleParameterAnnotations &left, CodeAttributeRuntimeInvisibleParameterAnnotations &right ) JVMX_NOEXCEPT
{
  JavaCodeAttribute::swap( left, right );
  std::swap( left.m_ParameterAnnotations, right.m_ParameterAnnotations );
}
