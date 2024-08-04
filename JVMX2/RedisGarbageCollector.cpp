#ifdef REDIS_SUPPORT

#include "OsFunctions.h"
#include "GlobalCatalog.h"

#include "InternalErrorException.h"

#include "RedisGarbageCollector.h"

RedisGarbageCollector::RedisGarbageCollector( const std::string &serverIpAddress, std::size_t portNo )
  : m_ServerIpAddress( serverIpAddress )
  , m_PortNo( portNo )
  , m_MustReconnect(false)
  , m_LastCollection( std::chrono::milliseconds(0) )
{
  m_MustReconnect = true;
  InnerConnect();
}

RedisGarbageCollector::~RedisGarbageCollector() noexcept
{
  m_MustReconnect = false;
  m_RedisClient.flushdb().quit().sync_commit();
 
  m_RedisClient.disconnect();

}

void RedisGarbageCollector::Init()
{
 
}

void RedisGarbageCollector::InnerConnect()
{
  if ( !m_RedisClient.is_connected() )
  {
    try
    {
      m_RedisClient.connect( m_ServerIpAddress, m_PortNo, [this]( const cpp_redis::redis_client& ) {OnDisconnected(); } );
    }
    catch ( cpp_redis::redis_error &ex )
    {
      throw InternalErrorException( ex.what() );
    }
    
    m_RedisClient.client_setname( GetClientName(), [this]( const cpp_redis::reply & reply ) {OnReply( reply ); } ).sync_commit();
  }
}

void *RedisGarbageCollector::AllocateBytes( size_t sizeInBytes )
{
  void *pNewObject = new char [sizeInBytes];

  //m_Pointers.push_back( pNewObject );

  return pNewObject;
}

void * RedisGarbageCollector::AllocateObject( size_t sizeInBytes )
{
  return AllocateBytes( sizeInBytes );
}

void * RedisGarbageCollector::AllocateArray( size_t sizeInBytes )
{
  return AllocateBytes( sizeInBytes );
}

void RedisGarbageCollector::Collect()
{
  m_LastCollection = std::chrono::system_clock::now();

  std::shared_ptr<IObjectRegistry> registry = GlobalCatalog::GetInstance().Get( "ObjectRegistry" );
  registry->Cleanup();
}

bool RedisGarbageCollector::MustCollect() const
{
  return std::chrono::system_clock::now() - m_LastCollection > std::chrono::seconds(15);
}

void RedisGarbageCollector::RunAllFinalizers( const std::shared_ptr<IVirtualMachineState> &pVMState )
{}

size_t RedisGarbageCollector::GetHeapSize() const
{
  return SIZE_MAX;
}

void RedisGarbageCollector::OnDisconnected()
{
  if ( m_MustReconnect )
  {
    InnerConnect();
  }
}

void RedisGarbageCollector::OnReply( const cpp_redis::reply &reply )
{
  switch ( reply.get_type())
  {
  case cpp_redis::reply::type::error: break;
    throw InternalErrorException( reply.error().c_str() );

  case cpp_redis::reply::type::bulk_string: break;
  case cpp_redis::reply::type::simple_string: break;
  case cpp_redis::reply::type::null: break;
  case cpp_redis::reply::type::integer: break;
  case cpp_redis::reply::type::array: break;
  default: break;
  }
}

const std::string& RedisGarbageCollector::GetClientName()
{
  if ( m_ClientName.empty() )
  {
    m_ClientName = "JVMX - ";
    m_ClientName += OsFunctions::GetInstance().GetHostName();
    m_ClientName += " - ";

    char buffer[ 65 ] = { 0 };
    m_ClientName += _ui64toa( OsFunctions::GetInstance().GetProcessID(), buffer , 10 );
  }

  return m_ClientName;
}

cpp_redis::redis_client &RedisGarbageCollector::GetRedisClient()
{
  return m_RedisClient;
}

#endif // REDIS_SUPPORT