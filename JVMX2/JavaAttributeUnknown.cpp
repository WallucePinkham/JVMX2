
#include "JavaAttributeUnknown.h"

JavaAttributeUnknown::JavaAttributeUnknown()
  : m_Data( DataBuffer::EmptyBuffer() )
{}

JavaAttributeUnknown::JavaAttributeUnknown( uint32_t length, const uint8_t *pBuffer )
  : m_Data( DataBuffer::FromByteArray( length, pBuffer ) )
{}

JavaAttributeUnknown::JavaAttributeUnknown( const JavaAttributeUnknown &other )
  : m_Data( other.m_Data )
{}

JavaAttributeUnknown::JavaAttributeUnknown( JavaAttributeUnknown &&other )
  : m_Data( DataBuffer::EmptyBuffer() )
{
  swap( *this, other );
}

JavaAttributeUnknown::~JavaAttributeUnknown() JVMX_NOEXCEPT
{
}

uint32_t JavaAttributeUnknown::GetLength() const JVMX_NOEXCEPT
{
  return m_Data.GetByteLength();
}

const uint8_t *JavaAttributeUnknown::GetValue() const JVMX_NOEXCEPT
{
  return m_Data.ToByteArray();
}

void JavaAttributeUnknown::swap( JavaAttributeUnknown &left, JavaAttributeUnknown &right ) JVMX_NOEXCEPT
{
  DataBuffer::swap( left.m_Data, right.m_Data );
}
