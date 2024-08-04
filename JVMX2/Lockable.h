#pragma once

#ifndef _LOCKABLE__H_
#define _LOCKABLE__H_

#include <memory>
#include <mutex>
#include <vector>
#include "GlobalConstants.h"


class Lockable
{
public:
  Lockable();
  virtual ~Lockable() JVMX_NOEXCEPT {};

  virtual void Lock( const char *pMethodName );
  virtual void Unlock( const char *pMethodName );

  inline virtual void lock() { Lock("wait"); }
  inline virtual void unlock() { Unlock("wait"); }

  size_t GetRecursionLevel() const;

private:
#ifdef _DEBUG
  uint32_t m_debugLockID;
  uint32_t m_debugChanged;

  std::vector<std::string> m_debugLockHistory;
  std::vector<std::string> m_debugUnlockHistory;
#endif // _DEBUG

  std::thread::id m_OwningThread;
  size_t m_RecursionLevel;

  std::recursive_mutex m_Monitor;
};

#endif // _LOCKABLE__H_