
#include "Lockable.h"
#include <future>

#define JVMX_DEBUG_LOCKS 0

static uint32_t s_nextDebugLockID = 1;

Lockable::Lockable() :
  m_OwningThread( std::thread::id() )
  , m_RecursionLevel( 0 )
{
#if defined( _DEBUG )
  m_debugLockID = s_nextDebugLockID ++;
  m_debugChanged = 0;
#endif // _DEBUG
}

void Lockable::Lock( const char *pMethodName )
{
#if defined( _DEBUG ) && defined (JVMX_DEBUG_LOCKS)
  ++ m_debugChanged;

  m_debugLockHistory.push_back( pMethodName );
#endif // _DEBUG

  if ( std::thread::id() == m_OwningThread )
  {
    m_RecursionLevel = 1;
  }
  else if ( m_OwningThread == std::this_thread::get_id() )
  {
    ++ m_RecursionLevel;
  }

  m_Monitor.lock();

  m_OwningThread = std::this_thread::get_id();
}

void Lockable::Unlock( const char *pMethodName )
{
#if defined( _DEBUG ) && defined (JVMX_DEBUG_LOCKS)
  ++ m_debugChanged;
  if ( std::thread::id() == m_OwningThread && m_OwningThread != std::this_thread::get_id() )
  {
    JVMX_ASSERT( false );
  }

  m_debugUnlockHistory.push_back( pMethodName );
#endif // _DEBUG

  if ( m_OwningThread == std::this_thread::get_id() )
  {
    if ( 0 != m_RecursionLevel )
    {
      -- m_RecursionLevel;
    }

    if ( 0 == m_RecursionLevel )
    {
      m_OwningThread = std::thread::id();
    }
  }

  m_Monitor.unlock();
}

size_t Lockable::GetRecursionLevel() const
{
  return m_RecursionLevel;
}
