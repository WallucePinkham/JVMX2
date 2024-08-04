
#ifndef _THREADINFO__H_
#define _THREADINFO__H_

#include <thread>
#include <memory>

#include <boost/intrusive_ptr.hpp>
#include <boost/thread/thread.hpp>

#include "ObjectReference.h"
#include "IVirtualMachineState.h"

class ThreadInfo
{
public:
  ThreadInfo();
  ThreadInfo( std::shared_ptr<boost::thread> pThread, boost::intrusive_ptr<ObjectReference> pThreadObject, std::shared_ptr<IVirtualMachineState> pVMState );
  ThreadInfo( const ThreadInfo &other );
  ThreadInfo &operator=( const ThreadInfo &other );

public:
  //std::shared_ptr<std::thread> m_pThread;
  std::shared_ptr<boost::thread> m_pThread;
  boost::intrusive_ptr<ObjectReference> m_pThreadObject;
  std::shared_ptr<IVirtualMachineState> m_pVMState;
};

#endif // _THREADINFO__H_

