#include <stdexcept>

#include "Stream.h"
#include "InvalidArgumentException.h"
#include "IndexOutOfBoundsException.h"

#include "CodeAttributeExceptions.h"

CodeAttributeExceptions::CodeAttributeExceptions( const CodeAttributeExceptions &other )
  : JavaCodeAttribute( other )
  , m_ExceptionIndices( other.m_ExceptionIndices )
{}

CodeAttributeExceptions::CodeAttributeExceptions( const JavaString &name, ExceptionTableIndexList list )
  : JavaCodeAttribute( name, e_JavaAttributeTypeExceptions )
  , m_ExceptionIndices( list )
{}

CodeAttributeExceptions::CodeAttributeExceptions( CodeAttributeExceptions &&other )
  : JavaCodeAttribute( JavaString::EmptyString(), e_JavaAttributeTypeExceptions )
{
  swap( *this, other );
}

CodeAttributeExceptions::~CodeAttributeExceptions()
{}

CodeAttributeExceptions CodeAttributeExceptions::FromBinary( JavaString name, Stream &byteStream )
{
  uint16_t numberOfExceptions = byteStream.ReadUint16();
  ExceptionTableIndexList list;

  for ( uint16_t i = 0; i < numberOfExceptions; ++ i )
  {
    list.push_back( byteStream.ReadUint16() );
  }

  return CodeAttributeExceptions( name, list );
}

void CodeAttributeExceptions::swap( CodeAttributeExceptions &left, CodeAttributeExceptions &right ) JVMX_NOEXCEPT
{
  JavaCodeAttribute::swap( left, right );
  std::swap( left.m_ExceptionIndices, right.m_ExceptionIndices );
}

DataBuffer CodeAttributeExceptions::ToBinary( const ConstantPool & ) const
{
  DataBuffer buffer = DataBuffer::EmptyBuffer();

  buffer = buffer.AppendUint16( static_cast<uint16_t>(m_ExceptionIndices.size()) );

  for (uint16_t index : m_ExceptionIndices)
  {
    buffer = buffer.AppendUint16( index );
  }

  return buffer;
}

bool CodeAttributeExceptions::Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT
{
  if ( other.GetType() != m_Type )
  {
    return false;
  }

  return Equals( static_cast<const CodeAttributeExceptions &>(other) );
}

bool CodeAttributeExceptions::Equals( const CodeAttributeExceptions &other ) const JVMX_NOEXCEPT
{
  return std::equal( m_ExceptionIndices.cbegin(), m_ExceptionIndices.cend(), other.m_ExceptionIndices.cbegin() );
}

uint16_t CodeAttributeExceptions::GetNumberOfExceptions() const
{
  return static_cast<uint16_t>(m_ExceptionIndices.size());
}

uint16_t CodeAttributeExceptions::GetIndexAt( size_t at ) const
{
  try
  {
    return m_ExceptionIndices.at( at );
  }
  catch ( std::out_of_range & )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Index out of bounds looking for exception table index." );
  }
}