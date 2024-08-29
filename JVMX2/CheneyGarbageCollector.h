
#ifndef _CHENEYGARBAGECOLLECTOR__H_
#define _CHENEYGARBAGECOLLECTOR__H_

#include <mutex>

#include "ThreadManager.h"
#include "IGarbageCollector.h"

enum class e_GarbageCollectionObjectTypes : uint8_t
{
  Invalid = 0,
  Object = 1,
  Array = 2,
  Bytes = 3
};

struct GCHeader;

class CheneyGarbageCollector : public IGarbageCollector, public std::enable_shared_from_this<CheneyGarbageCollector>
{
public:
  CheneyGarbageCollector( std::shared_ptr<IThreadManager> pThreadManager, size_t poolSizeInBytes );
  virtual ~CheneyGarbageCollector();

  bool IsPointerValid( void const * const pBytes ) const;

  virtual void *AllocateBytes( size_t sizeInBytes ) JVMX_OVERRIDE;
  virtual void *AllocateObject( size_t sizeInBytes ) JVMX_OVERRIDE;
  virtual void *AllocateArray( size_t sizeInBytes ) JVMX_OVERRIDE;
  virtual void RunAllFinalizers( const std::shared_ptr<IVirtualMachineState> & pVMState ) JVMX_OVERRIDE;
  virtual void Collect() JVMX_OVERRIDE;

  static void GetJavaLangClasses( std::vector<boost::intrusive_ptr<IJavaVariableType>> &roots );
  static void GetStringPoolRoots(std::vector<boost::intrusive_ptr<IJavaVariableType>>& roots);

  virtual size_t GetHeapSize() const JVMX_OVERRIDE;
  virtual bool MustCollect() const JVMX_OVERRIDE;

  virtual size_t GetFreeHeapSpace() const;

  virtual void AddRecentAllocation( boost::intrusive_ptr<ObjectReference> object ) JVMX_OVERRIDE;

private:
  void SwapSpaces();
  GCHeader *Copy( GCHeader *pHeader );
  IJavaVariableType *Copy( ObjectReference &object );

  static void CopyHeaderInternal( char * newObjectAddress, GCHeader * pHeader );
  static void CopyObjectInternal( GCHeader * pHeader, char * newObjectAddress );
  static void InitialiseObject( const GCHeader *pHeader, char * newObjectAddress );

  void *Allocate( size_t sizeInBytes, e_GarbageCollectionObjectTypes type );

  void CopyReferencesInArray( GCHeader *pHeader );
  void CopyObjectFields( GCHeader *pHeader );

  void CopyObjectFieldsInternal( JavaObject *pOldObject, std::shared_ptr<JavaClass> pClass );

  void RunFinalisersForOldObjects() const;

  void UpdatePointers();

  private:
  size_t m_PoolSizeInBytes;
  char *m_pMemoryPool;

  char *m_pAllocPtr;
  char *m_pScanPtr;

  char *m_pToSpace;
  char *m_pFromSpace;

  size_t m_AllocationCountSinceLastCollect;

private:
  struct OldToNewPointerMapping
  {
    ObjectReference pOld;
    IJavaVariableType *pNew;
  };

  std::vector<OldToNewPointerMapping> m_PointersToUpdate; 
  std::list<boost::intrusive_ptr<ObjectReference>> m_RecentAllocations;

#ifdef _DEBUG
  intptr_t m_debugReAllocBytes = 0;
  intptr_t m_debugSize = 0;
#endif // _DEBUG

  std::shared_ptr<IThreadManager> m_pThreadManager;

  std::recursive_mutex m_Mutex;

  bool m_IsCollecting;
};


#endif // _CHENEYGARBAGECOLLECTOR__H_

