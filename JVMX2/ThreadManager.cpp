

#include <chrono>

#include "IVirtualMachineState.h"

#include "ObjectReference.h"

#include "ThreadManager.h"
#include "GlobalCatalog.h"
#include "ILogger.h"

static const int c_MillisecondsToWaitForAllThreadsdToPause = 3000;
static const long c_SecondsToWaitForThreadJoin = 3;

ThreadInfo &ThreadManager::GetCurrentThreadInfo()
{
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );

  return m_JavaThreads[ boost::this_thread::get_id() ];
}

void ThreadManager::AddThread( std::shared_ptr<boost::thread> pNewThread, boost::intrusive_ptr<ObjectReference> pObject, std::shared_ptr<IVirtualMachineState> pNewState )
{
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );

  boost::thread::id id = boost::this_thread::get_id();

  if ( nullptr != pNewThread )
  {
    id = pNewThread->get_id();
  }

#ifdef _DEBUG
  JVMX_ASSERT( m_JavaThreads.cend() == m_JavaThreads.find( id ) );
#endif // _DEBUG

  m_JavaThreads[ id ] = ThreadInfo{ pNewThread, pObject, pNewState };
}

void ThreadManager::JoinAll()
{
  bool areThereJoinableThreadsLeft = false;
  for ( auto element : m_JavaThreads )
  {
    // Interrupt threads so they can shut down.
    element.second.m_pVMState->Interrupt();
  }

  // The purpose of the while loop is to keep trying if we get the error resource_deadlock_would_occur (EDEADLK)

  do
  {
    areThereJoinableThreadsLeft = JoinEachThread();
  }
  while ( areThereJoinableThreadsLeft );
}

void ThreadManager::DetachDaemons()
{
  bool moreElementsToCheck = true;

  do
  {
    moreElementsToCheck = false;

    for ( auto element : m_JavaThreads )
    {
      boost::intrusive_ptr<JavaBool> pIsDeamon = boost::dynamic_pointer_cast<JavaBool>( element.second.m_pThreadObject->GetContainedObject()->GetFieldByName( JavaString::FromCString( u"daemon" ) ) );
      if ( nullptr != pIsDeamon && pIsDeamon->ToBool() )
      {
        element.second.m_pThread->detach();
        m_JavaThreads.erase( element.first );
        moreElementsToCheck = true;
        break; // Break out of for loop
      }
    }
  }
  while ( moreElementsToCheck );
}


bool ThreadManager::JoinEachThread()
{
  bool areThereJoinableThreadsLeft = false;

  for ( auto element : m_JavaThreads )
  {
    if ( nullptr != element.second.m_pThread && element.second.m_pThread->joinable() )
    {
      areThereJoinableThreadsLeft = true;

      try
      {
        if ( element.second.m_pThread->joinable() )
        {
          bool joined = element.second.m_pThread->try_join_for( boost::chrono::seconds( c_SecondsToWaitForThreadJoin ) );

          if ( !joined )
          {
#ifdef _DEBUG
            std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
            pLogger->LogDebug( "Could not join thread (%s,%lld). Detaching.", element.second.m_pThreadObject->ToString().ToUtf8String().c_str(), element.second.m_pThread->get_id() );
#endif
            element.second.m_pThread->detach();
          }

        }
      }
      catch ( std::system_error &ex )
      {
        if ( ex.code() == std::errc::no_such_process )
        {
          JVMX_ASSERT( false ); // I would like to avoid this ever happening.
          m_JavaThreads.erase( element.first );

          break; // Break out of the for loop
        }
        else if ( ex.code() == std::errc::resource_deadlock_would_occur )
        {
          // Do nothing. The while loop in the caller should ensure that we come back to this thread and try again.
        }
        else
        {
          throw;
        }
      }
    }
  }

  return areThereJoinableThreadsLeft;
}

bool ThreadManager::WaitForThreadsToPause()
{
  bool allPaused = false;
  std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();

  while ( !allPaused )
  {
    allPaused = true;
    for ( auto element : m_JavaThreads )
    {
      if ( element.first != boost::this_thread::get_id() && !element.second.m_pVMState->IsPaused() )
      {
        if ( !element.second.m_pVMState->IsExecutingNative() )
        {
          allPaused = false;
        }
      }
    }

    std::chrono::system_clock::time_point endTime = std::chrono::system_clock::now();
    auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>( endTime - startTime );

    if ( !allPaused && int_ms.count() > c_MillisecondsToWaitForAllThreadsdToPause )
    {
      return false;
    }
  }

  return true;
}

void ThreadManager::PauseAllThreads()
{
  for ( auto element : m_JavaThreads )
  {
    if ( element.first != boost::this_thread::get_id() && !element.second.m_pVMState->IsPaused() )
    {
      element.second.m_pVMState->Pause();
    }
  }
}

void ThreadManager::ResumeAllThreads()
{
  for ( auto element : m_JavaThreads )
  {
    element.second.m_pVMState->Resume();
  }
}

std::vector<boost::intrusive_ptr<IJavaVariableType>> ThreadManager::GetRoots()
{
  std::vector<boost::intrusive_ptr<IJavaVariableType>> allRoots;

  for ( auto element : m_JavaThreads )
  {
    std::vector<boost::intrusive_ptr<IJavaVariableType>> roots = element.second.m_pVMState->GetGCRoots();
    allRoots.insert( allRoots.cend(), roots.cbegin(), roots.cend() );

    // We must only get the static objects once.
    if ( nullptr == element.second.m_pThread )
    {
      std::vector<boost::intrusive_ptr<IJavaVariableType>> staticVariables = element.second.m_pVMState->GetStaticObjectsAndArrays();
      allRoots.insert( allRoots.cend(), staticVariables.cbegin(), staticVariables.cend() );
    }
  }

  return allRoots;
}

std::shared_ptr<IVirtualMachineState> ThreadManager::GetCurrentThreadState()
{
#ifdef _DEBUG
  JVMX_ASSERT( m_JavaThreads.cend() != m_JavaThreads.find( boost::this_thread::get_id() ) );
#endif // _DEBUG

  return GetCurrentThreadInfo().m_pVMState;
}


