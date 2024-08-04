
#include "StackOverflowException.h"
#include "StackUnderrunException.h"

#include "BasicStackManager.h"

static const size_t c_DefaultStackSize = 1024;

BasicStackManager::BasicStackManager()
  : m_pBuffer( new uint8_t[ c_DefaultStackSize ] )
  , m_StackSize( c_DefaultStackSize )
  , m_UsedBytes( 0 )
{
#ifdef _DEBUG
  memset( m_pBuffer.get(), 0, m_StackSize );
#endif // _DEBUG
}

uint16_t BasicStackManager::PeekUint16() const
{
  return InternalPeek( static_cast<uint16_t>(0) );
}

uint16_t BasicStackManager::PopUint16()
{
  return InternalPop( static_cast<uint16_t>(0) );
}

void BasicStackManager::PushUint16( uint16_t value )
{
  InternalPush( value );
}

void BasicStackManager::PushPointer( uintptr_t value )
{
  InternalPush( value );
}

uintptr_t BasicStackManager::PopPointer()
{
  return InternalPop( static_cast<uintptr_t>(0) );
}

uintptr_t BasicStackManager::PeekPointer() const
{
  if ( m_UsedBytes < sizeof( uintptr_t ) )
  {
    throw StackUnderrunException( __FUNCTION__ " - Not enough bytes on the stack to peek at the next uint16." );
  }

  return *reinterpret_cast<uintptr_t *>(m_pBuffer.get() + m_UsedBytes - sizeof( uintptr_t ));
}

void BasicStackManager::AllocateBytes( uint16_t byteCount )
{
  if ( m_UsedBytes > m_StackSize - byteCount )
  {
    throw StackOverflowException( __FUNCTION__ " - Not enough space on the stack to allocate bytes. This stack manager will not resize the stack." );
  }

  m_UsedBytes += byteCount;
}

void BasicStackManager::FreeBytes( uint16_t byteCount )
{
  m_UsedBytes -= byteCount;
}

uint16_t BasicStackManager::GetUint16( uint16_t index ) const
{
  uint16_t byteIndex = index * sizeof( uint16_t );

  if ( m_UsedBytes < byteIndex )
  {
    throw StackUnderrunException( __FUNCTION__ " - Not enough bytes on the stack to get variable." );
  }

  return *reinterpret_cast<uint16_t *>(m_pBuffer.get() + m_UsedBytes - byteIndex);
}

void BasicStackManager::SetUint16( uint16_t index, uint16_t value )
{
  uint16_t byteIndex = index * sizeof( uint16_t );
  if ( m_UsedBytes < byteIndex )
  {
    throw StackUnderrunException( __FUNCTION__ " - Not enough bytes on the stack to get variable." );
  }

  uint16_t *pNextValue = reinterpret_cast<uint16_t *>(m_pBuffer.get() + m_UsedBytes - byteIndex);
  *pNextValue = value;
}

