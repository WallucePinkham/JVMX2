#include <stdexcept>

#include "Stream.h"
#include "InvalidArgumentException.h"
#include "IndexOutOfBoundsException.h"

#include "CodeAttributeLocalVariableTypeTable.h"

CodeAttributeLocalVariableTypeTable::CodeAttributeLocalVariableTypeTable( const CodeAttributeLocalVariableTypeTable &other )
  : JavaCodeAttribute( other )
  , m_LocalVariableTypes( other.m_LocalVariableTypes )
{}

CodeAttributeLocalVariableTypeTable::CodeAttributeLocalVariableTypeTable( const JavaString &name, LocalVariableTypeTable LocalVariables )
  : JavaCodeAttribute( name, e_JavaAttributeTypeLocalVariableTypeTable )
  , m_LocalVariableTypes( LocalVariables )
{}

CodeAttributeLocalVariableTypeTable::CodeAttributeLocalVariableTypeTable( CodeAttributeLocalVariableTypeTable &&other )
  : JavaCodeAttribute( JavaString::EmptyString(), e_JavaAttributeTypeLocalVariableTypeTable )
{
  swap( *this, other );
}

CodeAttributeLocalVariableTypeTable::~CodeAttributeLocalVariableTypeTable()
{}

CodeAttributeLocalVariableTypeTable CodeAttributeLocalVariableTypeTable::FromBinary( JavaString name, Stream &byteStream )
{
  uint16_t numberOfLocalVariables = byteStream.ReadUint16();
  LocalVariableTypeTable LocalVariables;

  for ( uint16_t i = 0; i < numberOfLocalVariables; ++ i )
  {
    uint16_t startPosition = byteStream.ReadUint16();
    uint16_t length = byteStream.ReadUint16();
    uint16_t nameIndex = byteStream.ReadUint16();
    uint16_t descriptorIndex = byteStream.ReadUint16();
    uint16_t index = byteStream.ReadUint16();

    LocalVariables.push_back( LocalVariableTypeTableEntry( startPosition, length, ConstantPoolStringReference::FromConstantPoolIndex( nameIndex ), descriptorIndex, index ) );
  }

  return CodeAttributeLocalVariableTypeTable( name, LocalVariables );
}

void CodeAttributeLocalVariableTypeTable::swap( CodeAttributeLocalVariableTypeTable &left, CodeAttributeLocalVariableTypeTable &right ) JVMX_NOEXCEPT
{
  JavaCodeAttribute::swap( left, right );
  std::swap( left.m_LocalVariableTypes, right.m_LocalVariableTypes );
}

DataBuffer CodeAttributeLocalVariableTypeTable::ToBinary( const ConstantPool & ) const
{
  DataBuffer buffer = DataBuffer::EmptyBuffer();

  buffer = buffer.AppendUint16( static_cast<uint16_t>(m_LocalVariableTypes.size()) );

  for (LocalVariableTypeTableEntry entry : m_LocalVariableTypes)
  {
    buffer = buffer.AppendUint16( entry.GetStartPosition() );
    buffer = buffer.AppendUint16( entry.GetLength() );
    buffer = buffer.AppendUint16( entry.GetNameReference()->ToConstantPoolIndex() );
    buffer = buffer.AppendUint16( entry.GetDescriptorIndex() );
    buffer = buffer.AppendUint16( entry.GetIndex() );
  }

  return buffer;
}

bool CodeAttributeLocalVariableTypeTable::Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT
{
  if ( other.GetType() != m_Type )
  {
    return false;
  }

  return Equals( static_cast<const CodeAttributeLocalVariableTypeTable &>(other) );
}

bool CodeAttributeLocalVariableTypeTable::Equals( const CodeAttributeLocalVariableTypeTable &other ) const JVMX_NOEXCEPT
{
  return std::equal( m_LocalVariableTypes.cbegin(), m_LocalVariableTypes.cend(), other.m_LocalVariableTypes.cbegin() );
}

uint16_t CodeAttributeLocalVariableTypeTable::GetNumberOfLocalVariableTypes() const
{
  return static_cast<uint16_t>(m_LocalVariableTypes.size());
}

uint16_t CodeAttributeLocalVariableTypeTable::GetStartPositionAt( size_t at ) const
{
  try
  {
    return m_LocalVariableTypes.at( at ).GetStartPosition();
  }
  catch ( std::out_of_range & )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Index out of bounds looking for local variable table index." );
  }
}

uint16_t CodeAttributeLocalVariableTypeTable::GetLengthAt( size_t at ) const
{
  try
  {
    return m_LocalVariableTypes.at( at ).GetLength();
  }
  catch ( std::out_of_range & )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Index out of bounds looking for local variable table index." );
  }
}

std::shared_ptr<ConstantPoolStringReference> CodeAttributeLocalVariableTypeTable::GetNameReferenceAt( size_t at ) const
{
  try
  {
    return m_LocalVariableTypes.at( at ).GetNameReference();
  }
  catch ( std::out_of_range & )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Index out of bounds looking for local variable table index." );
  }
}

uint16_t CodeAttributeLocalVariableTypeTable::GetSignatureIndexAt( size_t at ) const
{
  try
  {
    return m_LocalVariableTypes.at( at ).GetDescriptorIndex();
  }
  catch ( std::out_of_range & )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Index out of bounds looking for local variable table index." );
  }
}

uint16_t CodeAttributeLocalVariableTypeTable::GetIndexAt( size_t at ) const
{
  try
  {
    return m_LocalVariableTypes.at( at ).GetIndex();
  }
  catch ( std::out_of_range & )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Index out of bounds looking for local variable table index." );
  }
}