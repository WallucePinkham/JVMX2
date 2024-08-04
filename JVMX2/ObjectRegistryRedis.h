#pragma once

#ifndef _OBJECTREGISTRY_REDIS__H_
#define _OBJECTREGISTRY_REDIS__H_

#include <atomic>
#include <mutex>
#include <chrono>
#include <map>
#include <cpp_redis/cpp_redis>

#include "IObjectRegistry.h"
#include "GenericIterator.h"

class RedisGarbageCollector;

struct ObjectRegistryRedis_Entry
{
  std::chrono::time_point<std::chrono::system_clock> lastAccessTimePoint;
  IJavaVariableType *pObject;
  bool hasBeenUpdated;
#ifdef _DEBUG
  size_t size;
#endif // _DEBUG
};

class ObjectRegistryRedis : public IObjectRegistry
{
public:
  friend class ObjectReference;

private:
  typedef GenericIterator<std::map<ObjectIndexT, ObjectRegistryRedis_Entry>> Iterator;

public:
  ObjectRegistryRedis();
  virtual ~ObjectRegistryRedis() JVMX_NOEXCEPT JVMX_OVERRIDE {};

public:
  virtual size_t GetCount() const JVMX_OVERRIDE;

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

  std::shared_ptr<RedisGarbageCollector> GetGarbageCollector() const;

  static std::string ConvertObjectIndexToString( ObjectIndexT ref );

  void OnReplyDelete( const cpp_redis::reply &reply, ObjectIndexT ref );
  void OnReplyGet( const cpp_redis::reply &reply, ObjectIndexT ref );
  void OnReplySet( const cpp_redis::reply &reply, ObjectIndexT ref );

  void SetObject( JavaObject *pObject, size_t objectSizeInBytes, ObjectIndexT ref );

protected:
  mutable std::recursive_mutex m_Mutex;
  std::map<ObjectIndexT, ObjectRegistryRedis_Entry> m_Objects;
  std::atomic_intptr_t m_nextIndex;
};

#endif //_OBJECTREGISTRY_REDIS__H_
