#ifndef _BASICSTACKMANAGER__H_
#define _BASICSTACKMANAGER__H_

#include <memory>

#include "Stream.h"

#include "IStackManager.h"

class BasicStackManager : public IStackManager
{
public:
  BasicStackManager();
  virtual ~BasicStackManager() JVMX_NOEXCEPT {};

  virtual void PushUint16( uint16_t value ) JVMX_OVERRIDE;
  virtual uint16_t PopUint16() JVMX_OVERRIDE;
  virtual uint16_t PeekUint16() const JVMX_OVERRIDE;

  virtual void PushPointer( uintptr_t value ) JVMX_OVERRIDE;
  virtual uintptr_t PeekPointer() const JVMX_OVERRIDE;
  virtual uintptr_t PopPointer() JVMX_OVERRIDE;

  virtual void AllocateBytes( uint16_t byteCount ) JVMX_OVERRIDE;
  virtual void FreeBytes( uint16_t byteCount ) JVMX_OVERRIDE;

  virtual uint16_t GetUint16( uint16_t index ) const JVMX_OVERRIDE;
  virtual void SetUint16( uint16_t index, uint16_t value ) JVMX_OVERRIDE;

protected:
  template <typename T> T InternalPop( const T & );
  template <typename T> T InternalPeek( const T & ) const;
  template <typename T> void InternalPush( const T &value );

private:
  std::unique_ptr<uint8_t> m_pBuffer;
  size_t m_StackSize;
  size_t m_UsedBytes;
};

template <typename T>
T BasicStackManager::InternalPeek( const T & ) const
{
  if ( m_UsedBytes < sizeof( T ) )
  {
    throw StackUnderrunException( __FUNCTION__ " - Not enough bytes on the stack to peek at the next " JVMX_STRINGIZE( T ) );
  }

  return *reinterpret_cast<T *>(m_pBuffer.get() + m_UsedBytes - sizeof( T ));
}

template <typename T>
void BasicStackManager::InternalPush( const T &value )
{
  T *pNextValue = reinterpret_cast<T *>(m_pBuffer.get() + m_UsedBytes);
  AllocateBytes( sizeof( T ) );
  *pNextValue = value;
}

template <typename T>
T BasicStackManager::InternalPop( const T &value )
{
  T result = InternalPeek( value );
  FreeBytes( sizeof( T ) );
  return result;
}

#endif // _BASICSTACKMANAGER__H_
