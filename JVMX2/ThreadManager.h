#pragma once

#ifndef _JVMXTHREADMANAGER__H_
#define _JVMXTHREADMANAGER__H_

#include <memory>
#include <map>
#include <thread>

#include <boost/intrusive_ptr.hpp>

#include "ObjectReference.h"
#include "JavaClass.h"

#include "ThreadInfo.h"

#include "IThreadManager.h"

class IVirtualMachineState;

class ThreadManager : public IThreadManager
{
public:
  virtual void AddThread( std::shared_ptr<boost::thread> pNewThread, boost::intrusive_ptr<ObjectReference> pObject, std::shared_ptr<IVirtualMachineState> pNewState ) JVMX_OVERRIDE;
  virtual void JoinAll() JVMX_OVERRIDE;

  virtual void DetachDaemons() JVMX_OVERRIDE;

  virtual void PauseAllThreads() JVMX_OVERRIDE;
  virtual void ResumeAllThreads() JVMX_OVERRIDE;

  virtual ThreadInfo &GetCurrentThreadInfo() JVMX_OVERRIDE;

  virtual bool WaitForThreadsToPause() JVMX_OVERRIDE;

  virtual std::vector<boost::intrusive_ptr<IJavaVariableType>> GetRoots() JVMX_OVERRIDE;

  virtual std::shared_ptr<IVirtualMachineState> GetCurrentThreadState() JVMX_OVERRIDE;

private:
  bool JoinEachThread();

private:
  std::map<boost::thread::id, ThreadInfo> m_JavaThreads;
  std::recursive_mutex m_Mutex;
};

#endif // _JVMXTHREADMANAGER__H_
