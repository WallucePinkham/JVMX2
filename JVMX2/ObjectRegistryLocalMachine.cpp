
#include "IndexOutOfBoundsException.h"
#include "InvalidArgumentException.h"

#include "ObjectReference.h"
#include "ObjectRegistryLocalMachine.h"

// We don't want this to be 0, because we want to be able to detect invalid ObjectReferences with 0 as their index.
const intptr_t c_StartingIndex = 100;

size_t ObjectRegistryLocalMachine::GetCount() const
{
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );
  return m_Objects.size();
}

std::shared_ptr<const IIterator> ObjectRegistryLocalMachine::GetFirst() const
{
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );
  return std::make_shared<Iterator>( m_Objects.cbegin() );
}

bool ObjectRegistryLocalMachine::HasMore( const std::shared_ptr<const IIterator> &it ) const
{
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );

  std::shared_ptr<const Iterator> internalIterator = std::dynamic_pointer_cast<const Iterator>(it);
  return internalIterator->IsEnd( m_Objects );
}

std::shared_ptr<const IIterator> ObjectRegistryLocalMachine::GetNext( const std::shared_ptr<const IIterator> &it ) const
{
  return it->GetNext();
}

ObjectIndexT ObjectRegistryLocalMachine::GetIndexAt( const std::shared_ptr<const IIterator> &it ) const
{
  std::shared_ptr<const Iterator> internalIterator = std::dynamic_pointer_cast<const Iterator>(it);
  return internalIterator->Dereference().first;
}

bool ObjectRegistryLocalMachine::HasBeenUpdatedAt( const std::shared_ptr<const IIterator> &it ) const
{
  std::shared_ptr<const Iterator> internalIterator = std::dynamic_pointer_cast<const Iterator>(it);
  return internalIterator->Dereference().second.hasBeenUpdated;
}

ObjectRegistryLocalMachine::ObjectRegistryLocalMachine()
  : m_nextIndex( c_StartingIndex )
{
}

// ObjectRegistryLocalMachine &ObjectRegistryLocalMachine::GetInstance()
// {
//   if ( nullptr == m_pInstance )
//   {
//     m_pInstance = new ObjectRegistryLocalMachine;
//   }
// 
//   return *m_pInstance;
// }

ObjectReference ObjectRegistryLocalMachine::AddObject( JavaObject *pObject )
{
  ObjectIndexT ref = m_nextIndex++;

  std::lock_guard<std::recursive_mutex> lock( m_Mutex );
  m_Objects[ ref ] = { reinterpret_cast<IJavaVariableType *>(pObject), false };

  return ObjectReference( ref );
}

ObjectReference ObjectRegistryLocalMachine::AddObject( JavaArray *pArray )
{
  ObjectIndexT ref = m_nextIndex++;

  std::lock_guard<std::recursive_mutex> lock( m_Mutex );
  m_Objects[ ref ] = { reinterpret_cast<IJavaVariableType *>(pArray), false };

  return ObjectReference( ref );
}


void ObjectRegistryLocalMachine::RemoveObject( ObjectIndexT ref )
{
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );
  m_Objects.erase( ref );
}

void ObjectRegistryLocalMachine::UpdateObjectPointer( const ObjectReference &ref, IJavaVariableType *pObject )
{
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );

#ifdef _DEBUG
  if ( m_Objects.cend() == m_Objects.find( ref.GetIndex() ) )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected object to be found, before updating." );
  }
#endif // _DEBUG

  m_Objects[ ref.GetIndex() ].pObject = pObject;
  m_Objects[ ref.GetIndex() ].hasBeenUpdated = true;
}

void ObjectRegistryLocalMachine::Cleanup()
{
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );

  auto it = m_Objects.begin();
  while ( it != m_Objects.end() )
  {
    if ( it->second.hasBeenUpdated )
    {
      it->second.hasBeenUpdated = false;
    }
    else
    {
      it->second.pObject->~IJavaVariableType();
      m_Objects.erase( it++ );
      continue; // This is so that we don't increment again. Once we have erased, 
                // 'it' will be invalid and we don't want to increment again below.
    }

    ++ it;
  }
}

void ObjectRegistryLocalMachine::VerifyEntry( ObjectIndexT ref )
{
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );

  if ( 0 == ref )
  {
    return;
  }

  auto notFound = m_Objects.cend();
  auto pos = m_Objects.find( ref );

  if ( pos == notFound )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Requested object not found." );
  }

  char *pObj = (char *)(m_Objects[ ref ].pObject);

  int32_t baba = 0xbabababa;
  if (0 == memcmp(pObj, &baba, 4))
  {
    throw InvalidArgumentException( __FUNCTION__ " - Object not valid." );
  }

  int32_t cccc = 0xcccccccc;
  if ( 0 == memcmp( pObj, &cccc, 4 ) )
  //if ( *pObj == (char)0xcc )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Object not valid." );
  }
}

IJavaVariableType * ObjectRegistryLocalMachine::GetObject_( ObjectIndexT ref )
{
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );

#if defined( _DEBUG ) && defined (JVMX_DEBUG_OBJECTREF)
  auto notFound = m_Objects.cend();
  auto pos = m_Objects.find( ref );

  if ( pos == notFound )
  {
    pos = m_Objects.find( ref );

    if ( pos == notFound )
    {
      throw IndexOutOfBoundsException( __FUNCTION__ " - Requested object not found." );
    }
  }
#endif // _DEBUG

  return m_Objects[ ref ].pObject;
}


