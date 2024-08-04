
#ifndef _SIMPLEGREEDYMEMORYMANAGER__H_
#define _SIMPLEGREEDYMEMORYMANAGER__H_

#include <list>
#include <wallaroo/collaborator.h>

#include "IMemoryManager.h"

class ILogger; // Forward Declaration

class SimpleGreedyMemoryManager : public IMemoryManager
{
public:
  SimpleGreedyMemoryManager();
  virtual ~SimpleGreedyMemoryManager() JVMX_NOEXCEPT;

  virtual uint8_t *Allocate( size_t byteCount ) JVMX_OVERRIDE;
  virtual void Free( uint8_t *pBuffer ) JVMX_OVERRIDE;

  virtual size_t GetHeapSize() JVMX_OVERRIDE;
  virtual size_t GetFreeByteCount() JVMX_OVERRIDE;
  virtual size_t GetAllocatedByteCount() JVMX_OVERRIDE;

private:
  SimpleGreedyMemoryManager( const SimpleGreedyMemoryManager &other ) JVMX_FN_DELETE;
  SimpleGreedyMemoryManager( const SimpleGreedyMemoryManager &&other ) JVMX_FN_DELETE;
  SimpleGreedyMemoryManager &operator =(const SimpleGreedyMemoryManager &other) JVMX_FN_DELETE;

  void FreeAll();

private:
  std::list< std::pair<uint8_t *, size_t> > m_AllocatedBlocks;
  size_t m_allocatedByteCount;

  std::shared_ptr<ILogger> m_pLogger;
};

#endif // _SIMPLEGREEDYMEMORYMANAGER__H_
