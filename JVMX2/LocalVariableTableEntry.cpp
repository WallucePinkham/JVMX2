
#include "LocalVariableTableEntry.h"

LocalVariableTableEntry::LocalVariableTableEntry( uint16_t startPosition, uint32_t length, std::shared_ptr<ConstantPoolStringReference> pNameReference, uint16_t descriptorIndex, uint16_t index )
  : m_StartPosition( startPosition )
  , m_Length( length )
  , m_pNameReference( pNameReference )
  , m_DescriptorIndex( descriptorIndex )
  , m_Index( index )
{
}

LocalVariableTableEntry::LocalVariableTableEntry( const LocalVariableTableEntry &other )
  : m_pNameReference( ConstantPoolStringReference::FromConstantPoolIndex( 0 ) )
{
  *this = other;
}

LocalVariableTableEntry::~LocalVariableTableEntry() JVMX_NOEXCEPT
{
}

LocalVariableTableEntry &LocalVariableTableEntry::operator=(const LocalVariableTableEntry &other)
{
  if ( this != &other )
  {
    m_StartPosition = other.m_StartPosition;
    m_Length = other.m_Length;
    m_pNameReference = other.m_pNameReference;
    m_DescriptorIndex = other.m_DescriptorIndex;
    m_Index = other.m_Index;
  }

  return *this;
}


uint16_t LocalVariableTableEntry::GetStartPosition() const JVMX_NOEXCEPT
{
  return m_StartPosition;
}

uint32_t LocalVariableTableEntry::GetLength() const JVMX_NOEXCEPT
{
  return m_Length;
}

bool LocalVariableTableEntry::operator==(const LocalVariableTableEntry &other) const JVMX_NOEXCEPT
{
  return m_StartPosition == other.m_StartPosition &&
  m_Length == other.m_Length &&
  m_pNameReference == other.m_pNameReference &&
  m_DescriptorIndex == other.m_DescriptorIndex &&
  m_Index == other.m_Index;
}

std::shared_ptr<ConstantPoolStringReference> LocalVariableTableEntry::GetNameReference() const
{
  return m_pNameReference;
}

uint16_t LocalVariableTableEntry::GetDescriptorIndex() const JVMX_NOEXCEPT
{
  return m_DescriptorIndex;
}

uint16_t LocalVariableTableEntry::GetIndex() const JVMX_NOEXCEPT
{
  return m_Index;
}



