#include <exception>
#include <vector>
#include <set>

#include "GlobalConstants.h"
#include "JavaTypes.h"

#include "JavaArray.h"
#include "ObjectRegistryLocalMachine.h"

#include "GlobalCatalog.h"
#include "IJavaLangClassList.h"
#include "ILogger.h"
#include "OutOfMemoryException.h"
#include "InvalidStateException.h"

#include "CheneyGarbageCollector.h"
#include <cinttypes>

const JavaString c_FinalizeMethodName = JavaString::FromCString( JVMX_T( "finalize" ) );
const JavaString c_FinalizeMethodType = JavaString::FromCString( JVMX_T( "()V" ) );

const size_t c_MaxRecentAllocations = 100;

struct GCHeader
{
  e_GarbageCollectionObjectTypes type;
  size_t size;
  char *forwardingAddress;
};

CheneyGarbageCollector::CheneyGarbageCollector( std::shared_ptr<IThreadManager> pThreadManager, size_t poolSizeInBytes )
  : m_PoolSizeInBytes( poolSizeInBytes )
  , m_pMemoryPool( new char[ poolSizeInBytes ] )
  , m_AllocationCountSinceLastCollect( 0 )
  , m_pThreadManager( pThreadManager )
  , m_IsCollecting( false )
{
  //   initialize() =
  //     tospace = 0
  //     fromspace = N / 2
  //     allocPtr = tospace
  //     scanPtr = whatever -- only used during collection

  m_pToSpace = m_pMemoryPool;
  m_pFromSpace = m_pToSpace + ( m_PoolSizeInBytes / 2 );

  m_pAllocPtr = m_pToSpace;
  m_pScanPtr = m_pToSpace;
}

bool CheneyGarbageCollector::IsPointerValid( void const *const pBytes ) const
{
  return pBytes >= m_pMemoryPool && pBytes < m_pMemoryPool + m_PoolSizeInBytes;
}

void *CheneyGarbageCollector::AllocateBytes( size_t sizeInBytes )
{
  return Allocate( sizeInBytes, e_GarbageCollectionObjectTypes::Bytes );
}

void *CheneyGarbageCollector::AllocateObject( size_t sizeInBytes )
{
  return Allocate( sizeInBytes, e_GarbageCollectionObjectTypes::Object );
}

void *CheneyGarbageCollector::AllocateArray( size_t sizeInBytes )
{

  return Allocate( sizeInBytes, e_GarbageCollectionObjectTypes::Array );
}

void CheneyGarbageCollector::RunAllFinalizers( const std::shared_ptr<IVirtualMachineState> &pVMState )
{
#if defined(_DEBUG)
  {
    std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
    pLogger->LogDebug( "Garbage Collection Running All Finalizers..." );
  }
#endif // _DEBUG

  std::shared_ptr<IObjectRegistry> pObjectRegistry = GlobalCatalog::GetInstance().Get( "ObjectRegistry" );
  for ( auto i = pObjectRegistry->GetFirst(); pObjectRegistry->HasMore( i ); i = pObjectRegistry->GetNext( i ) )
  {
    boost::intrusive_ptr<ObjectReference> pObjectToFinalize = new ObjectReference( pObjectRegistry->GetIndexAt( i ) );
    if ( e_JavaVariableTypes::Object == pObjectToFinalize->GetVariableType() )
    {
      std::shared_ptr<MethodInfo> pMethodInfo = pVMState->ResolveMethod( pObjectToFinalize->GetContainedObject()->GetClass().get(), c_FinalizeMethodName, c_FinalizeMethodType );
      if ( nullptr != pMethodInfo )
      {
        pVMState->PushOperand( pObjectToFinalize );
        //pVMState->ExecuteMethod( *pObjectToFinalize->GetContainedObject()->GetClass()->GetName(), c_FinalizeMethodName, c_FinalizeMethodType, pMethodInfo );
        pVMState->ExecuteMethod( *pMethodInfo->GetClass()->GetName(), c_FinalizeMethodName, c_FinalizeMethodType, pMethodInfo );
      }
    }
  }
}

void *CheneyGarbageCollector::Allocate( size_t sizeInBytes, e_GarbageCollectionObjectTypes type )
{
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );
  ++ m_AllocationCountSinceLastCollect;

  //   allocate( n ) =
  //     If allocPtr + n > tospace + N / 2
  //     collect()
  //     EndIf
  //     If allocPtr + n > tospace + N / 2
  //     fail “insufficient memory”
  //     EndIf
  //     o = allocPtr
  //     allocPtr = allocPtr + n
  //     return o

  size_t finalSize = sizeInBytes + sizeof( GCHeader );

  if ( ( m_pAllocPtr + finalSize ) > m_pToSpace + ( m_PoolSizeInBytes / 2 ) )
  {
    throw OutOfMemoryException( __FUNCTION__ " - Out of memory." );
  }

  //   if ( (m_pAllocPtr + finalSize) > m_pToSpace + (m_PoolSizeInBytes / 2) )
  //   {
  //     throw std::exception( "Not enough memory." );
  //   }

  char *pResult = m_pAllocPtr;
  GCHeader *pHeader = reinterpret_cast<GCHeader *>( pResult );

  pResult += sizeof( GCHeader );
  pHeader->size = sizeInBytes;
  pHeader->type = type;
  pHeader->forwardingAddress = nullptr;

  m_pAllocPtr += finalSize;

  return static_cast<void *>( pResult );
}

CheneyGarbageCollector::~CheneyGarbageCollector()
{
  delete[] m_pMemoryPool;
}

void CheneyGarbageCollector::Collect()
{
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );

  if ( m_IsCollecting )
  {
    return;
  }

#ifdef _DEBUG
  m_debugSize = m_pAllocPtr - m_pToSpace;
  m_debugReAllocBytes = 0;
#endif // _DEBUG

  // We do this to handle a possible race condition. If two threads were blocking on m_Mutex,
  // then we want the first one to collect, and the second to just jump out of here.
  if ( m_AllocationCountSinceLastCollect < 10 )
  {
    return;
  }

#if defined(_DEBUG)
  {
    std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
    pLogger->LogDebug( "Garbage Collection Starting..." );
  }
#endif // _DEBUG

  //   collect() =
  //     swap( fromspace, tospace )
  //     allocPtr = tospace
  //     scanPtr = tospace
  //
  //     -- scan every root you've got
  //     ForEach root in the stack -- or elsewhere
  //     root = copy( root )
  //     EndForEach
  //
  //     -- scan objects in the heap( including objects added by this loop )
  //     While scanPtr < allocPtr
  //     ForEach reference r from o( pointed to by scanPtr )
  //     r = copy( r )
  //     EndForEach
  //     scanPtr = scanPtr + o.size() -- points to the next object in the heap, if any
  //     EndWhile

  m_IsCollecting = true;

  m_pThreadManager->PauseAllThreads();
  if ( !m_pThreadManager->WaitForThreadsToPause() )
  {
#if defined(_DEBUG)
    {
      std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
      pLogger->LogDebug( "Could not pause all threads, deferring Garbage Collection until later..." );
    }
#endif // _DEBUG

    m_IsCollecting = false;
    m_pThreadManager->ResumeAllThreads();

    return;
  }

  SwapSpaces();
  m_pAllocPtr = m_pToSpace;
  m_pScanPtr = m_pToSpace;

  try
  {
    std::vector<boost::intrusive_ptr<IJavaVariableType>> roots = m_pThreadManager->GetRoots();

    GetJavaLangClasses( roots );

    std::set<boost::intrusive_ptr<IJavaVariableType>> unqiueRoots;
    for ( auto it = roots.begin(); it != roots.end(); ++ it )
    {
      unqiueRoots.insert( *it );
    }

    // Make sure that we keep the last c_MaxRecentAllocations recent objects. This is because we sometimes allocate
    // objects in the C++ code, which the GC doesn't know is still being used.
    for ( auto it = m_RecentAllocations.begin(); it != m_RecentAllocations.end(); ++ it )
    {
      unqiueRoots.insert( *it );
    }

    for ( auto root : unqiueRoots )
    {
      boost::intrusive_ptr<ObjectReference> pRootObject = boost::dynamic_pointer_cast<ObjectReference>( root );

      //ObjectRegistry::GetInstance().UpdateObjectPointer( *pRootObject, Copy( pRootObject ) );
      IJavaVariableType *pResult = Copy( *pRootObject );
      m_PointersToUpdate.push_back( { *pRootObject, pResult } );
      //root = Copy( root );
    }

    while ( m_pScanPtr < m_pAllocPtr )
    {
      GCHeader *pHeader = reinterpret_cast<GCHeader *>( m_pScanPtr );
      if ( pHeader->type == e_GarbageCollectionObjectTypes::Object )
      {
        CopyObjectFields( pHeader );
      }
      else if ( pHeader->type == e_GarbageCollectionObjectTypes::Array )
      {
        CopyReferencesInArray( pHeader );
      }
      else if ( pHeader->type == e_GarbageCollectionObjectTypes::Bytes )
      {
        // Bytes can't contain references to objects.
      }
      else
      {
        throw InvalidStateException( __FUNCTION__ " - Unknown type of object in garbage collector." );
      }

      m_pScanPtr += pHeader->size + sizeof( GCHeader );
    }

    UpdatePointers();

    RunFinalisersForOldObjects();

    std::shared_ptr<IObjectRegistry> pObjectRegistry = GlobalCatalog::GetInstance().Get( "ObjectRegistry" );
    pObjectRegistry->Cleanup();
  }
  catch ( ... )
  {
    m_pThreadManager->ResumeAllThreads();
    m_IsCollecting = false;
    throw;
  }

  m_AllocationCountSinceLastCollect = 0;

#ifdef _DEBUG
  // Poison the space, so we can see what valid values are.
  memset( m_pFromSpace, 0xCC, m_PoolSizeInBytes / 2 );
#endif // _DEBUG

#if defined(_DEBUG)
  {
    std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
    pLogger->LogDebug( "Garbage Collection Complete. Resuming Threads..." );
  }
#endif // _DEBUG

  m_pThreadManager->ResumeAllThreads();
  m_IsCollecting = false;
}

void CheneyGarbageCollector::GetJavaLangClasses( std::vector<boost::intrusive_ptr<IJavaVariableType>> &roots )
{
  std::shared_ptr<IJavaLangClassList> pClassList = GlobalCatalog::GetInstance().Get( "JavaLangClassList" );
  for ( size_t i = 0; i < pClassList->GetCount(); ++i )
  {
    roots.push_back( pClassList->GetByIndex( i ) );
  }
}

void CheneyGarbageCollector::CopyObjectFields( GCHeader *pHeader )
{
  JavaObject *pOldObject = reinterpret_cast<JavaObject *>( reinterpret_cast<char *>( pHeader ) + sizeof( GCHeader ) );
  if ( pOldObject->GetVariableType() != e_JavaVariableTypes::Object )
  {
    throw InvalidStateException( __FUNCTION__ " - Pointer does not point to an object, as expected." );
  }

  CopyObjectFieldsInternal( pOldObject, pOldObject->GetClass() );

}

void CheneyGarbageCollector::CopyObjectFieldsInternal( JavaObject *pOldObject, std::shared_ptr<JavaClass> pClass )
{
  //for ( size_t i = 0; i < pOldObject->GetClass()->GetLocalFieldCount(); ++ i )
  for ( size_t i = 0; i < pClass->GetLocalFieldCount( e_PublicOnly::No ); ++ i )
  {
    auto pFieldInfo = pClass->GetFieldByIndex( i );

    if ( pFieldInfo->IsStatic() )
    {
      continue;
    }

    auto pTempField = pOldObject->GetFieldByName( *pFieldInfo->GetName() );
    const IJavaVariableType *pField = pTempField.get();

    //pOldObject->GetFieldByIndex( i );
    if ( e_JavaVariableTypes::Object == pField->GetVariableType() || e_JavaVariableTypes::Array == pField->GetVariableType() )
    {
      boost::intrusive_ptr<ObjectReference> pFieldObject = new ObjectReference( *dynamic_cast<const ObjectReference *>( pField ) );

      // Just record which pointers should be updated. Don't update them here yet.
      IJavaVariableType *pResult = Copy( *pFieldObject );
      m_PointersToUpdate.push_back( { *pFieldObject, pResult } );
    }
  }

  if ( nullptr != pClass->GetSuperClass() )
  {
    CopyObjectFieldsInternal( pOldObject, pClass->GetSuperClass() );
  }
}

void CheneyGarbageCollector::RunFinalisersForOldObjects() const
{
#if defined(_DEBUG)
  {
    std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
    pLogger->LogDebug( "Garbage Collection Running Finalizers..." );
  }
#endif // _DEBUG

  std::shared_ptr<IObjectRegistry> pObjectRegistry = GlobalCatalog::GetInstance().Get( "ObjectRegistry" );
  for ( auto pI = pObjectRegistry->GetFirst(); pObjectRegistry->HasMore( pI ); pI = pObjectRegistry->GetNext( pI ) )
  {
    if ( !pObjectRegistry->HasBeenUpdatedAt( pI ) )
    {
      std::shared_ptr<IVirtualMachineState>  pVMState = m_pThreadManager->GetCurrentThreadState();
      boost::intrusive_ptr<ObjectReference> pObjectToFinalize = new ObjectReference( pObjectRegistry->GetIndexAt( pI ) );
      if ( e_JavaVariableTypes::Object == pObjectToFinalize->GetVariableType() )
      {
        std::shared_ptr<MethodInfo> pMethodInfo = pVMState->ResolveMethod( pObjectToFinalize->GetContainedObject()->GetClass().get(), c_FinalizeMethodName, c_FinalizeMethodType );
        if ( nullptr != pMethodInfo )
        {
          pVMState->PushOperand( pObjectToFinalize );
          pVMState->ExecuteMethod( *pObjectToFinalize->GetContainedObject()->GetClass()->GetName(), c_FinalizeMethodName, c_FinalizeMethodType, pMethodInfo );
        }
      }
    }
  }
}

void CheneyGarbageCollector::UpdatePointers()
{
#if defined(_DEBUG)
  {
    std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
    pLogger->LogDebug( "Garbage Collection Updating Pointers..." );
  }
#endif // _DEBUG

  std::shared_ptr<IObjectRegistry> pObjectRegistry = GlobalCatalog::GetInstance().Get( "ObjectRegistry" );
  for ( auto it : m_PointersToUpdate )
  {
    pObjectRegistry->UpdateObjectPointer( it.pOld, it.pNew );
  }

  m_PointersToUpdate.clear();
}

void CheneyGarbageCollector::AddRecentAllocation( boost::intrusive_ptr<ObjectReference> object )
{
  if ( m_RecentAllocations.size() >= c_MaxRecentAllocations )
  {
    m_RecentAllocations.pop_back();
  }

  m_RecentAllocations.push_front( object );
}

void CheneyGarbageCollector::InitialiseObject( const GCHeader *pHeader, char *newObjectAddress )
{
  char *pFinalAddress = newObjectAddress + sizeof( GCHeader );

  if ( e_GarbageCollectionObjectTypes::Object == pHeader->type )
  {
    const JavaObject *pOldObject = reinterpret_cast<const JavaObject *>( reinterpret_cast<const char *>( pHeader ) + sizeof( GCHeader ) );
    new ( pFinalAddress ) JavaObject( pOldObject->GetClass() );
  }
  else if ( e_GarbageCollectionObjectTypes::Array == pHeader->type )
  {
    const JavaArray *pOldObject = reinterpret_cast<const JavaArray *>( reinterpret_cast<const char *>( pHeader ) + sizeof( GCHeader ) );
    new ( pFinalAddress ) JavaArray( pOldObject->GetContainedType(), pOldObject->GetNumberOfElements() );
  }
  else
  {
#ifdef _DEBUG
    memset( pFinalAddress, 0, pHeader->size );
#endif // _DEBUG
  }
}

void CheneyGarbageCollector::CopyReferencesInArray( GCHeader *pHeader )
{
  JavaArray *pArray = reinterpret_cast<JavaArray *>( reinterpret_cast<char *>( pHeader ) + sizeof( GCHeader ) );
  if ( pArray->GetVariableType() != e_JavaVariableTypes::Array )
  {
    throw InvalidStateException( __FUNCTION__ " - Pointer does not point to an array as expected." );
  }

  if ( pArray->GetContainedType() != e_JavaArrayTypes::Reference )
  {
    // Nothing to be done.
    return;
  }

  for ( size_t i = 0; i < pArray->GetNumberOfElements(); ++ i )
  {
    IJavaVariableType *pElement = pArray->At( i );
    if ( pElement->GetVariableType() == e_JavaVariableTypes::Object ||
         pElement->GetVariableType() == e_JavaVariableTypes::Array )
    {
      ObjectReference *pElementReference = dynamic_cast<ObjectReference *>( pElement );

      IJavaVariableType *pResult = Copy( *pElementReference );
      m_PointersToUpdate.push_back( { *pElementReference, pResult } );
    }
  }
}

void CheneyGarbageCollector::SwapSpaces()
{
  char *pTemp = m_pToSpace;
  m_pToSpace = m_pFromSpace;
  m_pFromSpace = pTemp;

#ifdef _DEBUG
  // Poison the space, so we can see what valid values are.
  memset( m_pToSpace, 0xBA, m_PoolSizeInBytes / 2 );
#endif // _DEBUG
}

IJavaVariableType *CheneyGarbageCollector::Copy( ObjectReference &object )
{
  char *pObjectStart = nullptr;
  if ( e_JavaVariableTypes::Object == object.GetVariableType() )
  {
    pObjectStart = reinterpret_cast<char *>( object.GetContainedObject() );
  }
  else if ( e_JavaVariableTypes::Array == object.GetVariableType() )
  {
    pObjectStart = reinterpret_cast<char *>( object.GetContainedArray() );
  }
  else
  {
    JVMX_ASSERT( false );
  }

  GCHeader *pOldHeader = reinterpret_cast<GCHeader *>( pObjectStart - sizeof( GCHeader ) );
  GCHeader *pHeader = Copy( pOldHeader );
  return reinterpret_cast<IJavaVariableType *>( reinterpret_cast<char *>( pHeader ) + sizeof( GCHeader ) );
}

GCHeader *CheneyGarbageCollector::Copy( GCHeader *pHeader )
{
  //   copy( o ) =
  //     If o has no forwarding address
  //     o' = allocPtr
  //     allocPtr = allocPtr + size( root )
  //     copy the contents of o to o'
  //     forwarding-address( o ) = o'
  //     EndIf
  //     return forwarding-address( o )

  if ( nullptr != pHeader->forwardingAddress )
  {
    return reinterpret_cast<GCHeader *>( pHeader->forwardingAddress );
  }

#ifdef _DEBUG
  if ( !( m_pAllocPtr + pHeader->size + sizeof( GCHeader ) < m_pMemoryPool + m_PoolSizeInBytes ) )
  {
    __asm int 3;
  }
#endif // _DEBUG

  char *newObjectAddress = m_pAllocPtr;

#ifdef _DEBUG
  m_debugReAllocBytes += pHeader->size + sizeof( GCHeader );
#endif // _DEBUG

  m_pAllocPtr += pHeader->size + sizeof( GCHeader );

  InitialiseObject( pHeader, newObjectAddress );
  CopyObjectInternal( pHeader, newObjectAddress );
  CopyHeaderInternal( newObjectAddress, pHeader );

  pHeader->forwardingAddress = newObjectAddress;

  return reinterpret_cast<GCHeader *>( pHeader->forwardingAddress );
}

void CheneyGarbageCollector::CopyHeaderInternal( char *newObjectAddress, GCHeader *pHeader )
{
  GCHeader *pNewHeader = reinterpret_cast<GCHeader *>( newObjectAddress );
  pNewHeader->size = pHeader->size;
  pNewHeader->type = pHeader->type;
  pNewHeader->forwardingAddress = nullptr;
}

void CheneyGarbageCollector::CopyObjectInternal( GCHeader *pHeader, char *newObjectAddress )
{
  if ( e_GarbageCollectionObjectTypes::Object == pHeader->type )
  {
    JavaObject *pOldObject = reinterpret_cast<JavaObject *>( reinterpret_cast<char *>( pHeader ) + sizeof( GCHeader ) );
    JavaObject *pNewObject = reinterpret_cast<JavaObject *>( newObjectAddress + sizeof( GCHeader ) );
    pNewObject->DeepClone( pOldObject );
  }
  else if ( e_GarbageCollectionObjectTypes::Array == pHeader->type )
  {
    JavaArray *pOldArray = reinterpret_cast<JavaArray *>( reinterpret_cast<char *>( pHeader ) + sizeof( GCHeader ) );
    JavaArray *pNewArray = reinterpret_cast<JavaArray *>( newObjectAddress + sizeof( GCHeader ) );
    pNewArray->DeepClone( pOldArray );
  }
  else
  {
    memcpy( newObjectAddress + sizeof( GCHeader ), reinterpret_cast< const char * >( pHeader ) + sizeof( GCHeader ), pHeader->size );
  }
}

size_t CheneyGarbageCollector::GetHeapSize() const
{
  return m_PoolSizeInBytes;
}

bool CheneyGarbageCollector::MustCollect() const
{
  if ( m_IsCollecting )
  {
    return false;
  }

  double x = static_cast<double>( GetHeapSize() ) / 2;
  double z = static_cast<double>( GetFreeHeapSpace() );

  double percentageSpaceLeft = ( static_cast<double>( GetFreeHeapSpace() ) / ( static_cast<double>( GetHeapSize() ) / static_cast<double>( 2 ) ) ) * 100.0;
  if ( percentageSpaceLeft < 10.0 )
  {
    return m_AllocationCountSinceLastCollect >= 100;
  }

  return false;
}

size_t CheneyGarbageCollector::GetFreeHeapSpace() const
{
  return ( m_pToSpace + ( m_PoolSizeInBytes / 2 ) ) - m_pAllocPtr;
}

