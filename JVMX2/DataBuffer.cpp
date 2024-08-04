
#include <cstring>
#include <algorithm>

#include "DataBuffer.h"
#include "InvalidArgumentException.h"
#include "HelperTypes.h"

DataBuffer::DataBuffer( size_t length, const uint8_t *pBuffer )
{
  InternalCopyOtherValue( pBuffer, length );
}

DataBuffer::DataBuffer( const DataBuffer &other )
{
  InternalCopyOtherValue( other.m_pBytes, other.m_Length );
}

DataBuffer::DataBuffer( DataBuffer &&other ) JVMX_NOEXCEPT
{
  m_pBytes = nullptr;
  m_Length = 0;

  swap( *this, other );
}

DataBuffer::~DataBuffer() JVMX_NOEXCEPT
{
  try
  {
    InternalCleanup();
  }
  catch ( ... )
  {
  }
}

DataBuffer DataBuffer::FromCString( const char16_t *pString )
{
  size_t byteLength = HelperTypes::String16Length( pString ) * sizeof( char16_t );

  return DataBuffer( byteLength, reinterpret_cast<const uint8_t *>(pString) );
}

const uint8_t *DataBuffer::ToByteArray() const
{
  return m_pBytes;
}

void DataBuffer::InternalCopyOtherValue( const uint8_t *pBuffer, size_t length )
{
  if ( nullptr == pBuffer )
  {
    throw InvalidArgumentException( "The string to contain was passed as NULL" );
  }

  m_pBytes = new uint8_t[ length ];
  m_Length = length;

  memcpy( m_pBytes, pBuffer, length );
}

void DataBuffer::InternalCleanup()
{
  delete [] m_pBytes;
  m_Length = 0;
}

DataBuffer &DataBuffer::operator=(DataBuffer other)
{
  swap( *this, other );

  return *this;
}

size_t DataBuffer::GetByteLength() const
{
  return m_Length;
}

void DataBuffer::swap( DataBuffer &left, DataBuffer &right ) JVMX_NOEXCEPT
{
  std::swap( left.m_Length, right.m_Length );
  std::swap( left.m_pBytes, right.m_pBytes );
}

bool DataBuffer::operator==(const DataBuffer &other) const
{
  if ( m_Length != other.m_Length )
  {
    return false;
  }

  return 0 == memcmp( m_pBytes, other.m_pBytes, m_Length );
}

const DataBuffer DataBuffer::EmptyBuffer()
{
  return DataBuffer::FromByteArray( 0, reinterpret_cast<const uint8_t *>(JVMX_T( "" )) );
}

DataBuffer DataBuffer::FromByteArray( size_t length, const uint8_t *pBuffer )
{
  return DataBuffer( length, pBuffer );
}

DataBuffer DataBuffer::Append( size_t length, const uint8_t *pBuffer ) const
{
  DataBuffer result = DataBuffer::EmptyBuffer();
  uint8_t *pResultBuffer = new uint8_t[ m_Length + length ];

  try
  {
    memcpy( pResultBuffer, m_pBytes, m_Length );
    memcpy( pResultBuffer + m_Length, pBuffer, length );

    result = DataBuffer::FromByteArray( m_Length + length, pResultBuffer );
  }
  catch ( ... )
  {
    delete[] pResultBuffer;
    throw;
  }

  delete[] pResultBuffer;

  return result;
}

DataBuffer DataBuffer::Append( const DataBuffer &other ) const
{
  return Append( other.m_Length, other.m_pBytes );
}

DataBuffer DataBuffer::AppendUint16( uint16_t value ) const
{
  return Append( sizeof( uint16_t ), reinterpret_cast<const uint8_t *>(&value) );
}

DataBuffer DataBuffer::AppendUint32( uint32_t value ) const
{
  return Append( sizeof( uint32_t ), reinterpret_cast<const uint8_t *>(&value) );
}

DataBuffer DataBuffer::AppendUint64( uint64_t value ) const
{
  return Append( sizeof( uint64_t ), reinterpret_cast<const uint8_t *>(&value) );
}

DataBuffer DataBuffer::AppendUint8( uint8_t value ) const
{
  return Append( sizeof( uint8_t ), &value );
}

bool DataBuffer::IsEmpty() const
{
  return 0 == m_Length;
}

const uint8_t * DataBuffer::GetRawDataPointer() const
{
  return m_pBytes;
}


