/*#include <memory>
#include <list>

#include "GlobalConstants.h"

#include "InvalidStateException.h"
#include "InvalidArgumentException.h"

#include "IJavaVariableType.h"
#include "IVirtualMachineState.h"

#include "JavaClass.h"
#include "JavaObject.h"

#include "GlobalCatalog.h"

#include "DefaultGarbageCollector.h"

const size_t c_MaxGeneration = 5;
const size_t c_MinGeneration = 1;

const JavaString c_FinalizeMethodName = JavaString::FromCString( JVMX_T( "finalize" ) );
const JavaString c_FinalizeMethodType = JavaString::FromCString( JVMX_T( "()V" ) );

DefaultGarbageCollector::DefaultGarbageCollector()
{}

DefaultGarbageCollector::~DefaultGarbageCollector()
{}

void DefaultGarbageCollector::Add( boost::intrusive_ptr<IJavaVariableType> pObject, e_AllowGarbageCollection allowCollection )
{
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );
  m_Entries.push_back( { pObject, allowCollection, c_MinGeneration, false, false } );
}

void DefaultGarbageCollector::Collect( e_ForceGarbageCollection force, const std::shared_ptr<IVirtualMachineState> & pVMState )
{
  CleanupGarbageObjects( force, pVMState );
  CleanupMemory();

  DetectCircularReferences( force );

  CleanupGarbageObjects( force, pVMState );
  CleanupMemory();

  //   while ( e_ForceGarbageCollection::Shutdown == force && !m_Entries.empty() )
  //   {
  //     CleanupGarbageObjects( force );
  //     CleanupMemory();
  //   }
}

void DefaultGarbageCollector::RunAllFinalizers( const std::shared_ptr<IVirtualMachineState> & pVMState )
{
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );

  for ( auto it = m_Entries.begin(); it != m_Entries.end(); ++ it )
  {
    if ( it->isDeleted || it->isFinalized )
    {
      continue;
    }

    FinaliseObject( it->pObject, pVMState );
    it->isFinalized = true;
  }
}

void DefaultGarbageCollector::CleanupGarbageObjects( e_ForceGarbageCollection force, const std::shared_ptr<IVirtualMachineState> & pVMState )
{
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );

  for ( auto it = m_Entries.begin(); it != m_Entries.end(); ++ it )
  {
    if ( e_AllowGarbageCollection::No == it->allowCollection && e_ForceGarbageCollection::Yes != force )
    {
      continue;
    }

    if ( it->isDeleted )
    {
      continue;
    }

    //     if ( e_JavaVariableTypes::Object == it->pObject->GetVariableType() )
    //     {
    //       std::set<boost::intrusive_ptr<JavaObject>> uniqueNodes;
    //
    //       boost::intrusive_ptr<JavaObject> pJavaObject = boost::dynamic_pointer_cast<JavaObject>(it->pObject);
    //       DetectCircularReferences( uniqueNodes, pJavaObject );
    //     }

    if ( nullptr != it->pObject && 1 == it->pObject->use_count() )
    {
      ++ it->generationCount;

      if ( e_ForceGarbageCollection::Yes == force )
      {
        it->generationCount = c_MaxGeneration;
      }
    }
    else
    {
      it->generationCount = c_MinGeneration;
    }

    if ( c_MaxGeneration <= it->generationCount )
    {
      if ( !it->isFinalized )
      {
        FinaliseObject( it->pObject, pVMState );
      }

      it->pObject.reset();
      it->isDeleted = true;
      it->isFinalized = true;
    }
  }
}

void DefaultGarbageCollector::CleanupMemory()
{
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );
  for ( auto it = m_Entries.begin(); it != m_Entries.end(); )
  {
    if ( it->isDeleted )
    {
      it = m_Entries.erase( it );
    }
    else
    {
      ++ it;
    }
  }
}

void DefaultGarbageCollector::DetectCircularReferencesInternal( std::set<IJavaVariableType *> &uniqueNodes, IJavaVariableType *pItem )
{
  if ( e_JavaVariableTypes::Object == pItem->GetVariableType() )
  {
    JavaObject *pJavaObject = dynamic_cast<JavaObject *>(pItem);

    CheckObjectFieldsForCircularReference( pJavaObject, uniqueNodes );
  }
  else if ( e_JavaVariableTypes::Array == pItem->GetVariableType() )
  {
    JavaArray *pJavaArray = dynamic_cast<JavaArray *>(pItem);

    if ( e_JavaArrayTypes::Reference != pJavaArray->GetContainedType() )
    {
      return;
    }

    CheckArrayElementsForCircularReference( pJavaArray, uniqueNodes );
  }
}

void DefaultGarbageCollector::CheckObjectFieldsForCircularReference( JavaObject * pJavaObject, std::set<IJavaVariableType *> &uniqueNodes )
{
  JavaObject::ObjectFields fields = pJavaObject->GetFields();
  for ( auto it = fields.begin(); it != fields.end(); ++ it )
  {
    if ( e_JavaVariableTypes::Object == it->second.pValue->GetVariableType() )
    {
      JavaObject *pCurrentField = dynamic_cast<JavaObject *>(it->second.pValue.get());
      if ( nullptr == pCurrentField )
      {
        continue;
      }

      if ( uniqueNodes.find( pCurrentField ) == uniqueNodes.end() )
      {
        uniqueNodes.insert( pCurrentField );
        DetectCircularReferencesInternal( uniqueNodes, pCurrentField );
      }
      else
      {
        // A circular reference is detected.
        it->second.pValue.reset();
      }
    }
    else if ( e_JavaVariableTypes::Array == it->second.pValue->GetVariableType() )
    {
      uniqueNodes.insert( it->second.pValue.get() );
      DetectCircularReferencesInternal( uniqueNodes, it->second.pValue.get() );
    }
  }
}

void DefaultGarbageCollector::CheckArrayElementsForCircularReference( JavaArray * pJavaArray, std::set<IJavaVariableType *> & uniqueNodes )
{
  for ( size_t it = 0; it < pJavaArray->GetNumberOfElements(); ++ it )
  {
    IJavaVariableType *pArrayElement = pJavaArray->At( it ).get();
    if ( e_JavaVariableTypes::Object == pArrayElement->GetVariableType() )
    {
      JavaObject *pJavaObject = dynamic_cast<JavaObject *>(pArrayElement);

      if ( uniqueNodes.find( pJavaObject ) == uniqueNodes.end() )
      {
        uniqueNodes.insert( pJavaObject );
        CheckObjectFieldsForCircularReference( pJavaObject, uniqueNodes );
      }
      else
      {
        // A circular reference is detected.
        pJavaArray->SetAt( JavaInteger::FromHostInt32( it ), boost::intrusive_ptr<JavaNullReference>( new JavaNullReference ) );
      }
    }
    else if ( e_JavaVariableTypes::Array == pArrayElement->GetVariableType() )
    {
      JavaArray *pElementReferencingAnArray = dynamic_cast<JavaArray *>(pArrayElement);

      if ( uniqueNodes.find( pElementReferencingAnArray ) == uniqueNodes.end() )
      {
        uniqueNodes.insert( pElementReferencingAnArray );
        CheckArrayElementsForCircularReference( pElementReferencingAnArray, uniqueNodes );
      }
      else
      {
        // A circular reference is detected.
        pJavaArray->SetAt( JavaInteger::FromHostInt32( it ), boost::intrusive_ptr<JavaNullReference>( new JavaNullReference ) );
      }
    }
  }
}

// void DefaultGarbageCollector::DetectCircularReferences( e_ForceGarbageCollection force )
// {
//   std::lock_guard<std::recursive_mutex> lock( m_Mutex );
//
//   // Kahn's Algorithm:
//   //   L <- Empty list that will contain the sorted elements
//   //     S <- Set of all nodes with no incoming edges
//   //     while S is non - empty do
//   //       remove a node n from S
//   //       add n to tail of L
//   //       for each node m with an edge e from n to m do
//   //         remove edge e from the graph
//   //         if m has no other incoming edges then
//   //           insert m into S
//   //   if graph has edges then
//   //     return error( graph has at least one cycle )
//   //   else
//   //     return L( a topologically sorted order )
//
//     // L <- Empty list that will contain the sorted elements
//   std::list<IJavaVariableType *> sortedList;
//
//   //S <- Set of all nodes with no incoming edges
//   std::list<IJavaVariableType *> objectsWithSingleRefrence;
//   for ( auto it = m_Entries.begin(); it != m_Entries.end(); ++ it )
//   {
//     if ( e_AllowGarbageCollection::No == it->allowCollection && e_ForceGarbageCollection::Yes != force )
//     {
//       continue;
//     }
//
//     if ( 2 == it->pObject->use_count() )
//     {
//       objectsWithSingleRefrence.push_front( it->pObject.get() );
//     }
//   }
//
//   //while S is non - empty do
//   while ( !objectsWithSingleRefrence.empty() )
//   {
//     //remove a node n from S
//     IJavaVariableType* pNodeN = objectsWithSingleRefrence.front();
//     objectsWithSingleRefrence.pop_front();
//
//     //add n to tail of L
//     sortedList.push_back( pNodeN );
//
//     // for each node m with an edge e from n to m do
//     for ( auto objectsIterator = m_Entries.begin(); objectsIterator != m_Entries.end(); ++ objectsIterator )
//     {
//       if ( e_JavaVariableTypes::Object == objectsIterator->pObject->GetVariableType() && e_JavaVariableTypes::Object == pNodeN->GetVariableType() )
//       {
//         boost::intrusive_ptr<JavaObject> pObjectM = (JavaObject *)objectsIterator->pObject.get();
//         boost::intrusive_ptr<JavaObject> pObjectN = (JavaObject *)pNodeN;
//
//         JavaObject::ObjectFields fields = pObjectN->GetFields();
//         for ( auto fieldsIterator = fields.begin(); fieldsIterator != fields.end(); ++ fieldsIterator )
//         {
//           if ( e_JavaVariableTypes::Object != fieldsIterator->second.pValue->GetVariableType() )
//           {
//             continue;
//           }
//
//           if ( fieldsIterator->second.pValue == pObjectM )
//           {
//             //remove edge e from the graph
//             fieldsIterator->second.pValue.reset();
//             if ( 1 == pObjectM->use_count() )
//             {
//               objectsWithSingleRefrence.push_back( pObjectM );
//             }
//           }
//         }
//       }
//     }
//   }
//
//   //           if graph has edges then
//   //             return error( graph has at least one cycle )
//   //           else
//   //             return L( a topologically sorted order )
// }

void DefaultGarbageCollector::DetectCircularReferences( e_ForceGarbageCollection force )
{
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );

  for each (auto entry in m_Entries)
  {
    if ( entry.isDeleted )
    {
      continue;
    }

    std::set<IJavaVariableType *> uniqueNodes;
    uniqueNodes.insert( entry.pObject.get() );

    DetectCircularReferencesInternal( uniqueNodes, entry.pObject.get() );
  }
}

void DefaultGarbageCollector::FinaliseObject( boost::intrusive_ptr<IJavaVariableType> pObject, const std::shared_ptr<IVirtualMachineState> & pVMState ) const
{
  if ( e_JavaVariableTypes::Object != pObject->GetVariableType() )
  {
    return;
  }

  // Todo.
  boost::intrusive_ptr<JavaObject> pJavaObject = boost::dynamic_pointer_cast<JavaObject>(pObject);
  if ( nullptr == pJavaObject )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected a valid object." );
  }

  pVMState->PushOperand( pJavaObject );

  auto pMethodInfo = pVMState->ResolveMethod( pJavaObject->GetClass().get(), c_FinalizeMethodName, c_FinalizeMethodType );
  pVMState->ExecuteMethod( *pJavaObject->GetClass()->GetName(), c_FinalizeMethodName, c_FinalizeMethodType, pMethodInfo );
}
*/