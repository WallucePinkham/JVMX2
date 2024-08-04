
#ifdef REDIS_SUPPORT

#include "ObjectReference.h"
#include "GlobalCatalog.h"

#include "RedisGarbageCollector.h"

#include "InvalidStateException.h"

#include "ObjectRegistryRedis.h"



ObjectRegistryRedis::ObjectRegistryRedis()
  : m_nextIndex( 100 )
{

}

size_t ObjectRegistryRedis::GetCount() const
{
  return m_Objects.size();
}

std::shared_ptr<const IIterator> ObjectRegistryRedis::GetFirst() const
{
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );
  return std::make_shared<Iterator>( m_Objects.cbegin() );
}

bool ObjectRegistryRedis::HasMore( const std::shared_ptr<const IIterator> &it ) const
{
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );

  std::shared_ptr<const Iterator> internalIterator = std::dynamic_pointer_cast<const Iterator>( it );
  return internalIterator->IsEnd( m_Objects );
}

std::shared_ptr<const IIterator> ObjectRegistryRedis::GetNext( const std::shared_ptr<const IIterator> &it ) const
{
  return it->GetNext();
}

ObjectIndexT ObjectRegistryRedis::GetIndexAt( const std::shared_ptr<const IIterator> &it ) const
{
  std::shared_ptr<const Iterator> internalIterator = std::dynamic_pointer_cast<const Iterator>( it );
  return internalIterator->Dereference().first;

}

bool ObjectRegistryRedis::HasBeenUpdatedAt( const std::shared_ptr<const IIterator> &it ) const
{
  std::shared_ptr<const Iterator> internalIterator = std::dynamic_pointer_cast<const Iterator>( it );
  return internalIterator->Dereference().second.hasBeenUpdated;

}

ObjectReference ObjectRegistryRedis::AddObject( JavaObject *pObject )
{
  ObjectIndexT ref = m_nextIndex++;
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );

  ObjectRegistryRedis_Entry entry = { std::chrono::system_clock::now(), reinterpret_cast<IJavaVariableType *>( pObject ), false };
#ifdef _DEBUG
  entry.size = pObject->GetSizeInBytes() + sizeof( JavaObject );
#endif // _DEBUG

  m_Objects[ ref ] = entry;



  SetObject( pObject, pObject->GetSizeInBytes() + sizeof( JavaObject ), ref );

  return ObjectReference( ref );
}

ObjectReference ObjectRegistryRedis::AddObject( JavaArray *pArray )
{
  ObjectIndexT ref = m_nextIndex++;
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );

  ObjectRegistryRedis_Entry entry = { std::chrono::system_clock::now(), reinterpret_cast<IJavaVariableType *>( pArray ), false };

#ifdef _DEBUG
  entry.size = pArray->CalculateSizeInBytes( pArray->GetContainedType(), pArray->GetNumberOfElements() ) + sizeof( JavaArray );
#endif // _DEBUG

  m_Objects[ ref ] = entry;

  SetObject( reinterpret_cast< JavaObject * >( pArray ), pArray->CalculateSizeInBytes( pArray->GetContainedType(), pArray->GetNumberOfElements() ) + sizeof( JavaArray ), ref );

  return ObjectReference( ref );
}

void ObjectRegistryRedis::SetObject( JavaObject *pObject, size_t objectSizeInBytes, ObjectIndexT ref )
{
  std::shared_ptr<RedisGarbageCollector> pRedisGarbageCollector = GetGarbageCollector();

  std::string objectAsString;
  objectAsString.append( reinterpret_cast<char *>( pObject ), objectSizeInBytes );

  cpp_redis::redis_client &redisClient = pRedisGarbageCollector->GetRedisClient();
  redisClient.set( { ConvertObjectIndexToString( ref ) }, objectAsString, [this, ref]( const cpp_redis::reply & reply )
  {
    OnReplySet( reply, ref );
  } );
  redisClient.sync_commit();
}

std::string ObjectRegistryRedis::ConvertObjectIndexToString( ObjectIndexT ref )
{
  std::stringstream refStream;
  refStream << ref;
  return refStream.str();
}

void ObjectRegistryRedis::OnReplyDelete( const cpp_redis::reply &reply, ObjectIndexT ref )
{
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );
  if ( 0 != reply.as_integer() )
  {
    ObjectRegistryRedis_Entry &entry = m_Objects.at( ref );
    if ( nullptr != entry.pObject )
    {
      char *pAsChar = reinterpret_cast<char *>( entry.pObject );
      delete[] pAsChar;
    }

    m_Objects.erase( ref );
  }
}

void ObjectRegistryRedis::OnReplyGet( const cpp_redis::reply &reply, ObjectIndexT ref )
{
  //  std::lock_guard<std::recursive_mutex> lock( m_Mutex );
  std::string value = reply.as_string();

  ObjectRegistryRedis_Entry &entry = m_Objects.at( ref );

  if ( nullptr != entry.pObject )
  {
    char *pAsChar = reinterpret_cast<char *>( entry.pObject );
    delete[] pAsChar;
  }

  entry.pObject = reinterpret_cast< IJavaVariableType * >( new char[ value.length() - 1 ] );
  memcpy( entry.pObject, value.c_str(), value.length() );
}

void ObjectRegistryRedis::OnReplySet( const cpp_redis::reply &reply, ObjectIndexT ref )
{
}

void ObjectRegistryRedis::RemoveObject( ObjectIndexT ref )
{
  std::shared_ptr<RedisGarbageCollector> pRedisGarbageCollector = GetGarbageCollector();

  std::lock_guard<std::recursive_mutex> lock( m_Mutex );

  cpp_redis::redis_client &redisClient = pRedisGarbageCollector->GetRedisClient();
  redisClient.del( { ConvertObjectIndexToString( ref ) }, [this, ref]( const cpp_redis::reply & reply )
  {
    OnReplyDelete( reply, ref );
  } );
  redisClient.commit();
}

std::shared_ptr<RedisGarbageCollector> ObjectRegistryRedis::GetGarbageCollector() const
{
  std::shared_ptr<IGarbageCollector> pGarbageCollector = GlobalCatalog::GetInstance().Get( "GarbageCollector" );
  std::shared_ptr<RedisGarbageCollector> pRedisGarbageCollector = std::dynamic_pointer_cast<RedisGarbageCollector>( pGarbageCollector );
  if ( nullptr == pRedisGarbageCollector )
  {
    JVMX_ASSERT( false );
    throw InvalidStateException( __FUNCTION__ " - Expected RedisGarbageCollector to be used with ObjectRegistryRedis." );
  }

  return pRedisGarbageCollector;
}

void ObjectRegistryRedis::UpdateObjectPointer( const ObjectReference &ref, IJavaVariableType *pObject )
{
}

void ObjectRegistryRedis::Cleanup()
{
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );

  for ( auto i = m_Objects.begin(); i != m_Objects.end(); ++ i )
  {
    if ( nullptr != i->second.pObject )
    {
      if ( std::chrono::system_clock::now() - i->second.lastAccessTimePoint > std::chrono::seconds( 30 ) )
      {
        if ( e_JavaVariableTypes::Array == i->second.pObject->GetVariableType() )
        {
          JavaArray *pArray = reinterpret_cast<JavaArray *>( i->second.pObject );
          size_t byteCount = sizeof( JavaArray ) + pArray->CalculateSizeInBytes( pArray->GetContainedType(), pArray->GetNumberOfElements() );
          SetObject( reinterpret_cast<JavaObject *>( i->second.pObject ), byteCount, i->first );
        }
        else
        {
          SetObject( reinterpret_cast<JavaObject *>( i->second.pObject ), reinterpret_cast<JavaObject *>( i->second.pObject )->GetSizeInBytes() + sizeof( JavaObject ), i->first );
        }

        //if ( std::chrono::system_clock::now() - i->second.lastAccessTimePoint > std::chrono::seconds( 30 ) )
        char *pAsChar = reinterpret_cast< char * >( i->second.pObject );
        delete[] pAsChar;
        i->second.pObject = nullptr;
      }
    }
  }
}

void ObjectRegistryRedis::VerifyEntry( ObjectIndexT ref )
{
}

IJavaVariableType *ObjectRegistryRedis::GetObject_( ObjectIndexT ref )
{
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );

  ObjectRegistryRedis_Entry &entry = m_Objects.at( ref );
  entry.lastAccessTimePoint = std::chrono::system_clock::now();

  if ( nullptr != entry.pObject )
  {
    return entry.pObject;
  }

  std::shared_ptr<RedisGarbageCollector> pRedisGarbageCollector = GetGarbageCollector();

  cpp_redis::redis_client &redisClient = pRedisGarbageCollector->GetRedisClient();
  redisClient.get( { ConvertObjectIndexToString( ref ) }, [this, ref]( const cpp_redis::reply & reply )
  {
    OnReplyGet( reply, ref );
  } );
  redisClient.sync_commit();

  JVMX_ASSERT( nullptr != entry.pObject );
  return entry.pObject;
}
#endif // REDIS_SUPPORT