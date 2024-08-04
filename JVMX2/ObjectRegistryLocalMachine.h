
#pragma once

#ifndef _OBJECTREGISTRY__H_
#define _OBJECTREGISTRY__H_

#include <atomic>
#include <map>
#include <mutex>

#include "IJavaVariableType.h"
#include "IObjectRegistry.h"
#include "GenericIterator.h"

class JavaArray;
class JavaObject;
class ObjectReference;

struct ObjectRegistryLocalMachine_Entry
{
  IJavaVariableType *pObject;
  bool hasBeenUpdated;
};

class ObjectRegistryLocalMachine : public IObjectRegistry
{
public:
  friend class ObjectReference;

public:
  ObjectRegistryLocalMachine();
  virtual ~ObjectRegistryLocalMachine() JVMX_NOEXCEPT JVMX_OVERRIDE {};

public:

    virtual size_t GetCount() const JVMX_OVERRIDE;

  //typedef std::map<ObjectIndexT, Entry>::const_iterator Iterator;
  typedef GenericIterator<std::map<ObjectIndexT, ObjectRegistryLocalMachine_Entry>> Iterator;

  virtual std::shared_ptr<const IIterator> GetFirst() const JVMX_OVERRIDE;
  virtual bool HasMore( const std::shared_ptr<const IIterator> &it ) const JVMX_OVERRIDE;
  virtual std::shared_ptr<const IIterator> GetNext( const std::shared_ptr<const IIterator> &it ) const JVMX_OVERRIDE;

  virtual ObjectIndexT GetIndexAt( const std::shared_ptr<const IIterator> &it ) const JVMX_OVERRIDE;
  virtual bool HasBeenUpdatedAt( const std::shared_ptr<const IIterator> &it ) const JVMX_OVERRIDE;

public:
  virtual ObjectReference AddObject( JavaObject *pObject ) JVMX_OVERRIDE;
  virtual ObjectReference AddObject( JavaArray *pArray ) JVMX_OVERRIDE;

  // For use by the Garbage Collector
  virtual void RemoveObject( ObjectIndexT ref ) JVMX_OVERRIDE;
  virtual void UpdateObjectPointer( const ObjectReference &ref, IJavaVariableType *pObject ) JVMX_OVERRIDE;

  virtual void Cleanup() JVMX_OVERRIDE;

  virtual void VerifyEntry( ObjectIndexT ref ) JVMX_OVERRIDE;

protected:
  virtual IJavaVariableType *GetObject_( ObjectIndexT ref ) JVMX_OVERRIDE;

private:
  mutable std::recursive_mutex m_Mutex;
  std::map<ObjectIndexT, ObjectRegistryLocalMachine_Entry> m_Objects;
  std::atomic_intptr_t m_nextIndex;
};

#endif // _OBJECTREGISTRY__H_

