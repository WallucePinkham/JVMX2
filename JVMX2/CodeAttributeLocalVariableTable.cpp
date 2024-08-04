#include <stdexcept>

#include "Stream.h"
#include "InvalidArgumentException.h"
#include "IndexOutOfBoundsException.h"

#include "CodeAttributeLocalVariableTable.h"

CodeAttributeLocalVariableTable::CodeAttributeLocalVariableTable( const CodeAttributeLocalVariableTable &other )
  : JavaCodeAttribute( other )
  , m_LocalVariables( other.m_LocalVariables )
{}

CodeAttributeLocalVariableTable::CodeAttributeLocalVariableTable( const JavaString &name, LocalVariableTable LocalVariables )
  : JavaCodeAttribute( name, e_JavaAttributeTypeLocalVariableTable )
  , m_LocalVariables( LocalVariables )
{}

CodeAttributeLocalVariableTable::CodeAttributeLocalVariableTable( CodeAttributeLocalVariableTable &&other )
  : JavaCodeAttribute( JavaString::EmptyString(), e_JavaAttributeTypeLocalVariableTable )
{
  swap( *this, other );
}

CodeAttributeLocalVariableTable::~CodeAttributeLocalVariableTable()
{}

CodeAttributeLocalVariableTable CodeAttributeLocalVariableTable::FromBinary( JavaString name, Stream &byteStream, const ConstantPool &constantPool )
{
  uint16_t numberOfLocalVariables = byteStream.ReadUint16();
  LocalVariableTable localVariables;

  localVariables.reserve( numberOfLocalVariables );

  for ( uint16_t i = 0; i < numberOfLocalVariables; ++ i )
  {
    uint16_t startPosition = byteStream.ReadUint16();
    uint32_t length = byteStream.ReadUint16();
    uint16_t nameIndex = byteStream.ReadUint16();
    uint16_t descriptorIndex = byteStream.ReadUint16();
    uint16_t index = byteStream.ReadUint16();

    std::shared_ptr<ConstantPoolStringReference> pName = ConstantPoolStringReference::FromConstantPoolIndex( nameIndex );
    pName->Prepare( &constantPool );

    localVariables.push_back( LocalVariableTableEntry( startPosition, length, pName, descriptorIndex, index ) );
  }

  return CodeAttributeLocalVariableTable( name, localVariables );
}

void CodeAttributeLocalVariableTable::swap( CodeAttributeLocalVariableTable &left, CodeAttributeLocalVariableTable &right ) JVMX_NOEXCEPT
{
  JavaCodeAttribute::swap( left, right );
  std::swap( left.m_LocalVariables, right.m_LocalVariables );
}

DataBuffer CodeAttributeLocalVariableTable::ToBinary( const ConstantPool & ) const
{
  DataBuffer buffer = DataBuffer::EmptyBuffer();

  buffer = buffer.AppendUint16( static_cast<uint16_t>(m_LocalVariables.size()) );

  for (LocalVariableTableEntry entry : m_LocalVariables)
  {
    buffer = buffer.AppendUint16( entry.GetStartPosition() );
    buffer = buffer.AppendUint16( static_cast<uint16_t>(entry.GetLength()) );
    buffer = buffer.AppendUint16( entry.GetNameReference()->ToConstantPoolIndex() );
    buffer = buffer.AppendUint16( entry.GetDescriptorIndex() );
    buffer = buffer.AppendUint16( entry.GetIndex() );
  }

  return buffer;
}

bool CodeAttributeLocalVariableTable::Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT
{
  if ( other.GetType() != m_Type )
  {
    return false;
  }

  return Equals( static_cast<const CodeAttributeLocalVariableTable &>(other) );
}

bool CodeAttributeLocalVariableTable::Equals( const CodeAttributeLocalVariableTable &other ) const JVMX_NOEXCEPT
{
  return std::equal( m_LocalVariables.cbegin(), m_LocalVariables.cend(), other.m_LocalVariables.cbegin() );
}

uint16_t CodeAttributeLocalVariableTable::GetNumberOfLocalVariables() const
{
  return static_cast<uint16_t>(m_LocalVariables.size());
}

uint16_t CodeAttributeLocalVariableTable::GetStartPositionAt( size_t at ) const
{
  try
  {
    return m_LocalVariables.at( at ).GetStartPosition();
  }
  catch ( std::out_of_range & )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Index out of bounds looking for local variable table index." );
  }
}

uint32_t CodeAttributeLocalVariableTable::GetLengthAt( size_t at ) const
{
  try
  {
    return m_LocalVariables.at( at ).GetLength();
  }
  catch ( std::out_of_range & )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Index out of bounds looking for local variable table index." );
  }
}

std::shared_ptr<ConstantPoolStringReference> CodeAttributeLocalVariableTable::GetNameReferenceAt( size_t at ) const
{
  try
  {
    return m_LocalVariables.at( at ).GetNameReference();
  }
  catch ( std::out_of_range & )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Index out of bounds looking for local variable table index." );
  }
}

uint16_t CodeAttributeLocalVariableTable::GetDescriptorIndexAt( size_t at ) const
{
  try
  {
    return m_LocalVariables.at( at ).GetDescriptorIndex();
  }
  catch ( std::out_of_range & )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Index out of bounds looking for local variable table index." );
  }
}

uint16_t CodeAttributeLocalVariableTable::GetIndexAt( size_t at ) const
{
  try
  {
    return m_LocalVariables.at( at ).GetIndex();
  }
  catch ( std::out_of_range & )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Index out of bounds looking for local variable table index." );
  }
}