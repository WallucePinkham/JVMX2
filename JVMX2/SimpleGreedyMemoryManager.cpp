
#include "GlobalConstants.h"

#include "ILogger.h"
#include "InvalidStateException.h"
#include "InvalidArgumentException.h"

#include "GlobalCatalog.h"

#include "SimpleGreedyMemoryManager.h"

const size_t c_FixedHeapSize = 100 * 1024 * 1024;
const size_t c_UnderflowDetection = SIZE_MAX - 1024;

SimpleGreedyMemoryManager::SimpleGreedyMemoryManager()
  : m_allocatedByteCount( 0 )
{
  m_pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
}

SimpleGreedyMemoryManager::~SimpleGreedyMemoryManager()
{
  try
  {
    FreeAll();
  }
  catch ( ... )
  {
    if ( nullptr != static_cast<std::shared_ptr<ILogger>>( m_pLogger ) )
    {
      m_pLogger->LogWarning( __FUNCTION__ " - Exception Caught in destructor" );
    }
  }
}

size_t SimpleGreedyMemoryManager::GetAllocatedByteCount()
{
  return m_allocatedByteCount;
}

size_t SimpleGreedyMemoryManager::GetFreeByteCount()
{
  return c_FixedHeapSize - m_allocatedByteCount;
}

size_t SimpleGreedyMemoryManager::GetHeapSize()
{
  return c_FixedHeapSize;
}

void SimpleGreedyMemoryManager::Free( uint8_t *pBuffer )
{
  for ( auto it = m_AllocatedBlocks.begin(); it != m_AllocatedBlocks.end(); ++ it )
  {
    if ( it->first == pBuffer )
    {
      size_t byteCount = it->second;

      JVMX_ASSERT( m_allocatedByteCount - byteCount < c_UnderflowDetection );
      if ( m_allocatedByteCount - byteCount > c_UnderflowDetection )
      {
        m_pLogger->LogWarning( __FUNCTION__ " - Freeing more bytes that we actually have allocated." );
        throw InvalidStateException( __FUNCTION__ " - Freeing more bytes that we actually have allocated." );
      }

      m_AllocatedBlocks.erase( it );
      delete[] pBuffer;

      m_allocatedByteCount -= byteCount;
      return;
    }
  }

  throw InvalidArgumentException( __FUNCTION__ " - Trying to free an invalid pointer." );
}

uint8_t *SimpleGreedyMemoryManager::Allocate( size_t byteCount )
{
  uint8_t *pResult = new uint8_t[ byteCount ];
  m_allocatedByteCount += byteCount;

  m_AllocatedBlocks.push_back( std::make_pair( pResult, byteCount ) );

  return pResult;
}

void SimpleGreedyMemoryManager::FreeAll()
{
  for ( auto pair : m_AllocatedBlocks )
  {
    delete[] pair.first;
  }

  m_AllocatedBlocks.clear();
  m_allocatedByteCount = 0;
}