#include "LocalVariableTypeTableEntry.h"


LocalVariableTypeTableEntry::LocalVariableTypeTableEntry( uint16_t startPosition, uint16_t length, std::shared_ptr<ConstantPoolStringReference> pNameReference, uint16_t descriptorIndex, uint16_t index )
  : m_StartPosition( startPosition )
  , m_Length( length )
  , m_pNameReference( pNameReference )
  , m_SignatureIndex( descriptorIndex )
  , m_Index( index )
{
}

LocalVariableTypeTableEntry::LocalVariableTypeTableEntry( const LocalVariableTypeTableEntry &other )
  : m_pNameReference( ConstantPoolStringReference::FromConstantPoolIndex( 0 ) )
{
  *this = other;
}

LocalVariableTypeTableEntry::~LocalVariableTypeTableEntry() JVMX_NOEXCEPT
{
}

LocalVariableTypeTableEntry &LocalVariableTypeTableEntry::
operator=(const LocalVariableTypeTableEntry &other)
{
  if ( this != &other )
  {
    m_StartPosition = other.m_StartPosition;
    m_Length = other.m_Length;
    m_pNameReference = other.m_pNameReference;
    m_SignatureIndex = other.m_SignatureIndex;
    m_Index = other.m_Index;
  }

  return *this;
}

bool LocalVariableTypeTableEntry::operator==(const LocalVariableTypeTableEntry &other) const JVMX_NOEXCEPT
{
  return m_StartPosition == other.m_StartPosition &&
  m_Length == other.m_Length &&
  m_pNameReference == other.m_pNameReference &&
  m_SignatureIndex == other.m_SignatureIndex &&
  m_Index == other.m_Index;
}

uint16_t LocalVariableTypeTableEntry::GetStartPosition() const JVMX_NOEXCEPT
{
  return m_StartPosition;
}

uint16_t LocalVariableTypeTableEntry::GetLength() const JVMX_NOEXCEPT
{
  return m_Length;
}

std::shared_ptr<ConstantPoolStringReference> LocalVariableTypeTableEntry::GetNameReference() const
{
  return m_pNameReference;
}

uint16_t LocalVariableTypeTableEntry::GetDescriptorIndex() const JVMX_NOEXCEPT
{
  return m_SignatureIndex;
}

uint16_t LocalVariableTypeTableEntry::GetIndex() const JVMX_NOEXCEPT
{
  return m_Index;
}
