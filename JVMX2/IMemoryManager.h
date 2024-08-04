
#ifndef _IMEMORYMANAGER__H_
#define _IMEMORYMANAGER__H_

#include "GlobalConstants.h"

#include <wallaroo/part.h>

class IMemoryManager JVMX_ABSTRACT : public wallaroo::Part
{
public:
  virtual ~IMemoryManager() JVMX_NOEXCEPT{};

  virtual uint8_t *Allocate( size_t byteCount ) JVMX_PURE;
  virtual void Free( uint8_t *pBuffer ) JVMX_PURE;

  virtual size_t GetHeapSize() JVMX_PURE;
  virtual size_t GetFreeByteCount() JVMX_PURE;
  virtual size_t GetAllocatedByteCount() JVMX_PURE;

protected:
  IMemoryManager() {};
  IMemoryManager( const IMemoryManager &other ) JVMX_FN_DELETE ;
  IMemoryManager &operator==(const IMemoryManager &other) JVMX_FN_DELETE;
};

#endif // _IMEMORYMANAGER__H_


