
#include "GlobalConstants.h"

#include "ILogger.h"
#include "InvalidArgumentException.h"

#include "GlobalCatalog.h"

#include "MallocFreeMemoryManager.h"

const size_t c_FixedHeapSize = 100 * 1024 * 1024;
const size_t c_UnderflowDetection = SIZE_MAX - 1024;

MallocFreeMemoryManager::MallocFreeMemoryManager()
  : m_allocatedByteCount( 0 )
{
}

MallocFreeMemoryManager::~MallocFreeMemoryManager()
{
  try
  {
    FreeAll();
  }
  catch ( ... )
  {
    std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
    if ( nullptr != pLogger )
    {
      pLogger->LogWarning( __FUNCTION__ " - Exception Caught in destructor" );
    }
  }
}

size_t MallocFreeMemoryManager::GetAllocatedByteCount()
{
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );
  return m_allocatedByteCount;
}

size_t MallocFreeMemoryManager::GetFreeByteCount()
{
  return c_FixedHeapSize - m_allocatedByteCount;
}

size_t MallocFreeMemoryManager::GetHeapSize()
{
  return c_FixedHeapSize;
}

void MallocFreeMemoryManager::Free( uint8_t *pBuffer )
{
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );

  if ( nullptr == pBuffer )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Trying to free an null pointer." );
  }

  auto pos = m_AllocatedBlocks.find( pBuffer );
  if ( m_AllocatedBlocks.end() == pos )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Trying to free an invalid pointer." );
  }

  m_allocatedByteCount -= pos->second;
  m_AllocatedBlocks.erase( pos );

  free( pBuffer );
}

uint8_t *MallocFreeMemoryManager::Allocate( size_t byteCount )
{
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );

  if ( 0 == byteCount )
  {
    byteCount = 1;
  }

  uint8_t *pBuffer = static_cast<uint8_t *>( malloc( byteCount ) );

  m_allocatedByteCount += byteCount;
  m_AllocatedBlocks[ pBuffer ] = byteCount;

  return pBuffer;
}

void MallocFreeMemoryManager::FreeAll()
{
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );

  for ( auto i = m_AllocatedBlocks.begin(); i != m_AllocatedBlocks.end(); ++ i )
  {
    free( i->first );
  }

  m_AllocatedBlocks.clear();
  m_allocatedByteCount = 0;
}