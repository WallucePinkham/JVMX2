
#include "ThreadInfo.h"

ThreadInfo &ThreadInfo::operator=( const ThreadInfo &other )
{
  JVMX_ASSERT( nullptr == m_pThread );

  m_pThread = other.m_pThread;
  m_pThreadObject = other.m_pThreadObject;
  m_pVMState = other.m_pVMState;

  JVMX_ASSERT( nullptr != m_pVMState );

  return *this;
}

ThreadInfo::ThreadInfo( const ThreadInfo &other )
{
  m_pThread = other.m_pThread;
  m_pThreadObject = other.m_pThreadObject;
  m_pVMState = other.m_pVMState;

  JVMX_ASSERT( nullptr != m_pVMState );
}

ThreadInfo::ThreadInfo( std::shared_ptr<boost::thread> pThread, boost::intrusive_ptr<ObjectReference> pThreadObject, std::shared_ptr<IVirtualMachineState> pVMState ) : m_pThread( pThread )
  , m_pThreadObject( pThreadObject )
  , m_pVMState( pVMState )
{
  JVMX_ASSERT( nullptr != m_pVMState );
}

ThreadInfo::ThreadInfo()
{

}

