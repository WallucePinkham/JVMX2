
#include <stdexcept>

#include "IndexOutOfBoundsException.h"
#include "CodeAttributeRuntimeInvisibleAnnotations.h"

CodeAttributeRuntimeInvisibleAnnotations::CodeAttributeRuntimeInvisibleAnnotations( const JavaString &name, AnnotationsList annotations )
  : JavaCodeAttribute( name, e_JavaAttributeTypeRuntimeInvisibleAnnotations )
{
  m_Annotations = annotations;
}

CodeAttributeRuntimeInvisibleAnnotations::CodeAttributeRuntimeInvisibleAnnotations( const CodeAttributeRuntimeInvisibleAnnotations &other )
  : JavaCodeAttribute( other )
{
  this->m_Annotations = other.m_Annotations;
}

CodeAttributeRuntimeInvisibleAnnotations::CodeAttributeRuntimeInvisibleAnnotations( CodeAttributeRuntimeInvisibleAnnotations &&other )
  : JavaCodeAttribute( JavaString::EmptyString(), e_JavaAttributeTypeRuntimeInvisibleAnnotations )
{
  swap( *this, other );
}

bool CodeAttributeRuntimeInvisibleAnnotations::Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT
{
  if ( other.GetType() != m_Type )
  {
    return false;
  }

  return Equals( static_cast<const CodeAttributeRuntimeInvisibleAnnotations &>(other) );
}

bool CodeAttributeRuntimeInvisibleAnnotations::Equals( const CodeAttributeRuntimeInvisibleAnnotations &other ) const JVMX_NOEXCEPT
{
  return std::equal( m_Annotations.cbegin(), m_Annotations.cend(), other.m_Annotations.cbegin() );
}

uint16_t CodeAttributeRuntimeInvisibleAnnotations::GetNumberOfAnnotations() const JVMX_NOEXCEPT
{
  return static_cast<uint16_t>(m_Annotations.size());
}

std::shared_ptr<AnnotationsEntry> CodeAttributeRuntimeInvisibleAnnotations::GetAnnotationAt( size_t at ) const
{
  try
  {
    return m_Annotations.at( at );
  }
  catch ( std::out_of_range & )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Index out of bounds looking for exception table index." );
  }
}

CodeAttributeRuntimeInvisibleAnnotations::~CodeAttributeRuntimeInvisibleAnnotations() JVMX_NOEXCEPT
{
}

CodeAttributeRuntimeInvisibleAnnotations CodeAttributeRuntimeInvisibleAnnotations::FromBinary( JavaString name, Stream &byteStream )
{
  AnnotationsList annotations;
  uint16_t count = byteStream.ReadUint16();

  for ( uint16_t index = 0; index < count; ++ index )
  {
    annotations.push_back( AnnotationsEntry::FromBinary( byteStream ) );
  }

  return CodeAttributeRuntimeInvisibleAnnotations( name, annotations );
}

DataBuffer CodeAttributeRuntimeInvisibleAnnotations::ToBinary( const ConstantPool & ) const
{
  DataBuffer buffer = DataBuffer::EmptyBuffer();

  buffer = buffer.AppendUint16( static_cast<uint16_t>(m_Annotations.size()) );
  for ( size_t index = 0; index < m_Annotations.size(); ++ index )
  {
    buffer = buffer.Append( m_Annotations.at( index )->ToBinary() );
  }

  return buffer;
}

void CodeAttributeRuntimeInvisibleAnnotations::swap( CodeAttributeRuntimeInvisibleAnnotations &left, CodeAttributeRuntimeInvisibleAnnotations &right ) JVMX_NOEXCEPT
{
  JavaCodeAttribute::swap( left, right );
  std::swap( left.m_Annotations, right.m_Annotations );
}
