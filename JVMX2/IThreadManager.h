
// IThreadManager
#ifndef _ITHREADMANAGER__H_
#define _ITHREADMANAGER__H_

#include <boost/intrusive_ptr.hpp>
#include <boost/thread/thread.hpp>

#include <wallaroo/part.h>
#include "GlobalConstants.h"

class ThreadInfo;
class ObjectReference;
class IJavaVariableType;
class IVirtualMachineState;

class IThreadManager : public wallaroo::Part
{
public:
  virtual ~IThreadManager() JVMX_NOEXCEPT {};

  virtual void AddThread( std::shared_ptr<boost::thread> pNewThread, boost::intrusive_ptr<ObjectReference> pObject, std::shared_ptr<IVirtualMachineState> pNewState ) JVMX_PURE;
  virtual void JoinAll() JVMX_PURE;
  virtual void DetachDaemons() JVMX_PURE;

  virtual void PauseAllThreads() JVMX_PURE;
  virtual void ResumeAllThreads() JVMX_PURE;
  virtual bool WaitForThreadsToPause() JVMX_PURE;

  virtual ThreadInfo &GetCurrentThreadInfo() JVMX_PURE;

  virtual std::vector<boost::intrusive_ptr<IJavaVariableType>> GetRoots() JVMX_PURE;

  virtual std::shared_ptr<IVirtualMachineState> GetCurrentThreadState() JVMX_PURE;
};



#endif // _ITHREADMANAGER__H_


