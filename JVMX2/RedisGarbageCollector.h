#pragma once

#ifndef _REDISGARBAGECOLLECTOR__H_
#define _REDISGARBAGECOLLECTOR__H_

#include <mutex>
#include <cpp_redis\cpp_redis>

#include "ThreadManager.h"
#include "IGarbageCollector.h"

class RedisGarbageCollector : public IGarbageCollector, public std::enable_shared_from_this<RedisGarbageCollector>
{
  friend class ObjectRegistryRedis;

public:
  RedisGarbageCollector( const std::string &serverIpAddress, std::size_t portNo = 6379 );

  virtual ~RedisGarbageCollector() JVMX_NOEXCEPT;;

public:
  virtual void Init() JVMX_OVERRIDE;

  virtual void * AllocateBytes( size_t sizeInBytes ) JVMX_OVERRIDE;
  virtual void * AllocateObject( size_t sizeInBytes ) JVMX_OVERRIDE;
  virtual void * AllocateArray( size_t sizeInBytes ) JVMX_OVERRIDE;
  virtual void Collect() JVMX_OVERRIDE;
  virtual bool MustCollect() const JVMX_OVERRIDE;
  virtual void RunAllFinalizers( const std::shared_ptr<IVirtualMachineState> &pVMState ) JVMX_OVERRIDE;
  virtual size_t GetHeapSize() const JVMX_OVERRIDE;

  virtual void AddRecentAllocation( boost::intrusive_ptr<ObjectReference> object ) JVMX_OVERRIDE {};

private:
  void OnDisconnected();
  void OnReply(const cpp_redis::reply &reply);
  void InnerConnect();

  const std::string& GetClientName();
 
private:
  cpp_redis::redis_client m_RedisClient;

  std::string m_ServerIpAddress;
  std::size_t m_PortNo;
  bool m_MustReconnect;

  std::string m_ClientName;
  std::chrono::time_point<std::chrono::system_clock> m_LastCollection;

protected:
  cpp_redis::redis_client &GetRedisClient();
};


#endif // _REDISGARBAGECOLLECTOR__H_
