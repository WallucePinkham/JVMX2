#include <stdexcept>

#include "Stream.h"
#include "InvalidArgumentException.h"
#include "IndexOutOfBoundsException.h"

#include "CodeAttributeLineNumberTable.h"

CodeAttributeLineNumberTable::CodeAttributeLineNumberTable( const CodeAttributeLineNumberTable &other )
  : JavaCodeAttribute( other )
  , m_LineNumbers( other.m_LineNumbers )
{}

CodeAttributeLineNumberTable::CodeAttributeLineNumberTable( const JavaString &name,
  LineNumberTable lineNumbers )
  : JavaCodeAttribute( name, e_JavaAttributeTypeLineNumberTable )
  , m_LineNumbers( lineNumbers )
{}

CodeAttributeLineNumberTable::CodeAttributeLineNumberTable( CodeAttributeLineNumberTable &&other )
  : JavaCodeAttribute( JavaString::EmptyString(), e_JavaAttributeTypeLineNumberTable )
{
  swap( *this, other );
}

CodeAttributeLineNumberTable::~CodeAttributeLineNumberTable()
{}

CodeAttributeLineNumberTable CodeAttributeLineNumberTable::FromBinary( JavaString name, Stream &byteStream )
{
  uint16_t numberOfLineNumbers = byteStream.ReadUint16();
  LineNumberTable lineNumbers;

  lineNumbers.reserve( numberOfLineNumbers );

  for ( uint16_t i = 0; i < numberOfLineNumbers; ++ i )
  {
    uint16_t startPosition = byteStream.ReadUint16();
    uint16_t lineNumber = byteStream.ReadUint16();

    lineNumbers.push_back( LineNumberTableEntry( startPosition, lineNumber ) );
  }

  return CodeAttributeLineNumberTable( name, lineNumbers );
}

void CodeAttributeLineNumberTable::swap( CodeAttributeLineNumberTable &left, CodeAttributeLineNumberTable &right ) JVMX_NOEXCEPT
{
  JavaCodeAttribute::swap( left, right );
  std::swap( left.m_LineNumbers, right.m_LineNumbers );
}

DataBuffer CodeAttributeLineNumberTable::ToBinary( const ConstantPool & ) const
{
  DataBuffer buffer = DataBuffer::EmptyBuffer();

  buffer = buffer.AppendUint16( static_cast<uint16_t>(m_LineNumbers.size()) );

  for (LineNumberTableEntry entry : m_LineNumbers)
  {
    buffer = buffer.AppendUint16( entry.GetStartPosition() );
    buffer = buffer.AppendUint16( entry.GetLineNumber() );
  }

  return buffer;
}

bool CodeAttributeLineNumberTable::Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT
{
  if ( other.GetType() != m_Type )
  {
    return false;
  }

  return Equals( static_cast<const CodeAttributeLineNumberTable &>(other) );
}

bool CodeAttributeLineNumberTable::Equals( const CodeAttributeLineNumberTable &other ) const JVMX_NOEXCEPT
{
  return std::equal( m_LineNumbers.cbegin(), m_LineNumbers.cend(), other.m_LineNumbers.cbegin() );
}

uint16_t CodeAttributeLineNumberTable::GetNumberOfLineNumbers() const
{
  return static_cast<uint16_t>(m_LineNumbers.size());
}

uint16_t CodeAttributeLineNumberTable::GetLineNumberAt( size_t at ) const
{
  try
  {
    return m_LineNumbers.at( at ).GetLineNumber();
  }
  catch ( std::out_of_range & )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Index out of bounds looking for exception table index." );
  }
}

uint16_t CodeAttributeLineNumberTable::GetStartPositionAt( size_t at ) const
{
  try
  {
    return m_LineNumbers.at( at ).GetStartPosition();
  }
  catch ( std::out_of_range & )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Index out of bounds looking for exception table index." );
  }
}