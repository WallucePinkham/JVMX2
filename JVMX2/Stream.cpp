#include "IndexOutOfBoundsException.h"

#include "Stream.h"

Stream::Stream( const DataBuffer &buffer ) 
  : m_Buffer( buffer )
  , m_StreamPosition( 0 )
{}

// Stream::Stream( Stream &&other ) JVMX_NOEXCEPT 
//   : m_Buffer( std::move( other.m_Buffer ) )
// {
//   std::swap( m_StreamPosition, other.m_StreamPosition );
// }

Stream::Stream( const Stream &other )
  : m_Buffer( other.m_Buffer )
  , m_StreamPosition( other.m_StreamPosition )
{}

Stream Stream::FromByteArray( size_t length, const uint8_t *pBuffer )
{
  return FromDataBuffer( DataBuffer::FromByteArray( length, pBuffer ) );
}

Stream Stream::FromDataBuffer( const DataBuffer &buffer )
{
  return Stream( buffer );
}

Stream::~Stream() JVMX_NOEXCEPT
{}

size_t Stream::GetByteLength() const JVMX_NOEXCEPT
{
  return m_Buffer.GetByteLength();
}

const uint8_t *Stream::ToByteArray() const JVMX_NOEXCEPT
{
  return m_Buffer.ToByteArray();
}

void Stream::swap( Stream &left, Stream &right ) JVMX_NOEXCEPT
{
  DataBuffer::swap( left.m_Buffer, right.m_Buffer );
  std::swap( left.m_StreamPosition, right.m_StreamPosition );
}

Stream & Stream::operator=( Stream other )
{
  swap( *this, other );

  return *this;
}

uint16_t Stream::ReadUint16()
{
  return ReadFromBuffer<uint16_t>();
}

uint32_t Stream::ReadUint32()
{
  return ReadFromBuffer<uint32_t>();
}

uint64_t Stream::ReadUint64()
{
  return ReadFromBuffer<uint64_t>();
}

DataBuffer Stream::ReadBytes( size_t length )
{
  if ( m_StreamPosition + length > m_Buffer.GetByteLength() )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Trying to read past the end of the buffer." );
  }

  DataBuffer result = DataBuffer::FromByteArray( length, m_Buffer.ToByteArray() + m_StreamPosition );

  m_StreamPosition += length;

  return result;
}

Stream Stream::EmptyStream()
{
  return Stream::FromDataBuffer( DataBuffer::EmptyBuffer() );
}

uint8_t Stream::ReadUint8()
{
  return ReadFromBuffer<uint8_t>();
}

bool Stream::IsAtEnd() const JVMX_NOEXCEPT
{
  return 0 == GetBytesLeft();
}

size_t Stream::GetBytesLeft() const JVMX_NOEXCEPT
{
  return m_Buffer.GetByteLength() - m_StreamPosition;
}

DataBuffer Stream::ToDataBuffer() const
{
  if ( m_StreamPosition > m_Buffer.GetByteLength() )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Trying to convert an invalid stream to a buffer." );
  }

  const uint8_t *pBuffer = m_Buffer.ToByteArray();

  return DataBuffer::FromByteArray( m_Buffer.GetByteLength() - m_StreamPosition, pBuffer + m_StreamPosition );
}