#ifndef _DEFAULTGARBAGECOLLECTOR__H_
#define _DEFAULTGARBAGECOLLECTOR__H_

#include <mutex>
#include <list>
#include <set>

#include <wallaroo/collaborator.h>

#include "IClassLibrary.h"
#include "JavaNativeInterface.h"

// class IVirtualMachineState;
// 
// class IMemoryManager;
// 
// class JavaArray;
// 
// #include "IGarbageCollector.h"
// 
// class DefaultGarbageCollector : public IGarbageCollector
// {
// public:
//   DefaultGarbageCollector();
//   virtual ~DefaultGarbageCollector();
// 
//   virtual void Add( boost::intrusive_ptr<IJavaVariableType> pObject, e_AllowGarbageCollection allowCollection ) JVMX_OVERRIDE;
// 
//   virtual void Collect( e_ForceGarbageCollection force, const std::shared_ptr<IVirtualMachineState> & pVMState ) JVMX_OVERRIDE;
// 
//   virtual void RunAllFinalizers( const std::shared_ptr<IVirtualMachineState> & pVMState ) JVMX_OVERRIDE;
// 
// private:
//   DefaultGarbageCollector( const DefaultGarbageCollector & )JVMX_FN_DELETE;
//   DefaultGarbageCollector( DefaultGarbageCollector && ) JVMX_FN_DELETE;
// 
//   DefaultGarbageCollector &operator=( const DefaultGarbageCollector & )JVMX_FN_DELETE;
//   DefaultGarbageCollector &operator=( DefaultGarbageCollector && )JVMX_FN_DELETE;
// 
//   bool operator==( const DefaultGarbageCollector & )JVMX_FN_DELETE;
//   bool operator<( const DefaultGarbageCollector & )JVMX_FN_DELETE;
// 
// private:
//   void FinaliseObject( boost::intrusive_ptr<IJavaVariableType> pObject, const std::shared_ptr<IVirtualMachineState> & pVMState ) const;
// 
//   void CleanupGarbageObjects( e_ForceGarbageCollection force, const std::shared_ptr<IVirtualMachineState> & pVMState );
//   void CleanupMemory();
//   void DetectCircularReferences( e_ForceGarbageCollection force );
// 
//   //void ListTopLevelNodesAsUnique( std::set<boost::intrusive_ptr<JavaObject>> &uniqueNodes );
// 
//   void DetectCircularReferencesInternal( std::set<IJavaVariableType *> &uniqueNodes, IJavaVariableType *pJavaObject );
// 
//   void CheckObjectFieldsForCircularReference( JavaObject *pJavaObject, std::set<IJavaVariableType *> &uniqueNodes );
//   void CheckArrayElementsForCircularReference( JavaArray *pJavaArray, std::set<IJavaVariableType *> & uniqueNodes );
// 
// private:
//   struct GarbageEntry
//   {
//     boost::intrusive_ptr<IJavaVariableType> pObject;
//     e_AllowGarbageCollection allowCollection;
//     size_t generationCount;
//     bool isFinalized;
//     bool isDeleted;
//   };
// 
//   std::list<GarbageEntry> m_Entries;
// 
//   std::recursive_mutex m_Mutex;
// };

#endif // _DEFAULTGARBAGECOLLECTOR__H_
