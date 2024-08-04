#ifndef _MALLOCFREEMEMORYMANAGER__H_
#define _MALLOCFREEMEMORYMANAGER__H_

#include <unordered_map>
#include <memory>
#include <mutex>

#include "ILogger.h"

#include <wallaroo/collaborator.h>

#include "IMemoryManager.h"

class ILogger; // Forward Declaration

class MallocFreeMemoryManager : public IMemoryManager
{
public:
  MallocFreeMemoryManager();
  virtual ~MallocFreeMemoryManager() JVMX_NOEXCEPT;

  virtual uint8_t *Allocate( size_t byteCount ) JVMX_OVERRIDE;
  virtual void Free( uint8_t *pBuffer ) JVMX_OVERRIDE;

  virtual size_t GetHeapSize() JVMX_OVERRIDE;
  virtual size_t GetFreeByteCount() JVMX_OVERRIDE;
  virtual size_t GetAllocatedByteCount() JVMX_OVERRIDE;

private:
  MallocFreeMemoryManager( const MallocFreeMemoryManager &other );
  MallocFreeMemoryManager( const MallocFreeMemoryManager &&other );
  MallocFreeMemoryManager &operator =( const MallocFreeMemoryManager &other );

  void FreeAll();

private:
  std::unordered_map< uint8_t *, size_t> m_AllocatedBlocks;
  size_t m_allocatedByteCount;

  std::recursive_mutex m_Mutex;
};

#endif // _MALLOCFREEMEMORYMANAGER__H_
