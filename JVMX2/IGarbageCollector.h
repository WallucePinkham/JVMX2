
#ifndef _IGARBAGECOLLECTOR__H_
#define _IGARBAGECOLLECTOR__H_

#include "GlobalConstants.h"

#include <wallaroo/part.h>


class JavaObject;
class JavaClass;
class IMemoryManager;
class IJavaVariableType;
class IVirtualMachineState;

enum class e_AllowGarbageCollection : uint16_t
{
  No,
  Yes = 1,
};

class IGarbageCollector : public wallaroo::Part
{
public:
  virtual ~IGarbageCollector() {}

  virtual void *AllocateBytes( size_t sizeInBytes ) JVMX_PURE;
  virtual void *AllocateObject( size_t sizeInBytes ) JVMX_PURE;
  virtual void *AllocateArray( size_t sizeInBytes ) JVMX_PURE;


//   virtual boost::intrusive_ptr<JavaObject> AllocateObject( std::shared_ptr<IMemoryManager> pMemoryManager, std::shared_ptr<JavaClass> pClass );
//   virtual boost::intrusive_ptr<ObjectReference> AllocateArray( std::shared_ptr<IMemoryManager> pMemoryManager, e_JavaArrayTypes type, size_t size );
//   virtual void Add( boost::intrusive_ptr<IJavaVariableType> pObject, e_AllowGarbageCollection allowCollection ) JVMX_PURE;
//   virtual void Collect( e_ForceGarbageCollection force, const std::shared_ptr<IVirtualMachineState> & pVMState ) JVMX_PURE;

  virtual void Collect() JVMX_PURE;

  virtual bool MustCollect() const JVMX_PURE;

  virtual void RunAllFinalizers( const std::shared_ptr<IVirtualMachineState> & pVMState ) JVMX_PURE;

  virtual size_t GetHeapSize() const JVMX_PURE;

  virtual void AddRecentAllocation( boost::intrusive_ptr<ObjectReference> object ) JVMX_PURE;

protected:
  IGarbageCollector() {};
};

#endif // _IGARBAGECOLLECTOR__H_
