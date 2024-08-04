
#ifndef _ISTACKMANAGER__H_
#define _ISTACKMANAGER__H_

#include "GlobalConstants.h"

class IStackManager //JVMX_ABSTRACT
{
public:
  virtual ~IStackManager() {}

  virtual void PushUint16( uint16_t value ) JVMX_PURE;
  virtual uint16_t PopUint16() JVMX_PURE;
  virtual uint16_t PeekUint16() const JVMX_PURE;

  virtual void PushPointer( uintptr_t value ) JVMX_PURE;
  virtual uintptr_t PeekPointer() const JVMX_PURE;
  virtual uintptr_t PopPointer() JVMX_PURE;

  virtual void AllocateBytes( uint16_t byteCount ) JVMX_PURE;
  virtual void FreeBytes( uint16_t byteCount ) JVMX_PURE;

  // These functions are used to set / get local varaibles. They do *not* allocate or free memory.
  // It is assumed that the memory is already allocated via AllocateBytes() and FreeBytes();
  virtual uint16_t GetUint16( uint16_t index ) const JVMX_PURE;
  virtual void SetUint16( uint16_t index, uint16_t value ) JVMX_PURE;

protected:
  IStackManager() {}
};

#endif // _ISTACKMANAGER__H_
