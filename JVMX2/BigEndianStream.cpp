
#include <sstream>

#include "Endian.h"

#include "BigEndianStream.h"

BigEndianStream::BigEndianStream( const Stream &contained )
  : Stream( contained )
{
}

// BigEndianStream::BigEndianStream( Stream &&contained )
//   : Stream( std::move( contained ) )
// {
// }

DataBuffer BigEndianStream::ReadBytes( size_t length )
{
  return Stream::ReadBytes( length );
}

uint64_t BigEndianStream::ReadUint64()
{
  return Endian::ntohll( Stream::ReadUint64() );
}

uint32_t BigEndianStream::ReadUint32()
{
  return Endian::ntohl( Stream::ReadUint32() );
}

uint16_t BigEndianStream::ReadUint16()
{
  return Endian::ntohs( Stream::ReadUint16() );
}

size_t BigEndianStream::GetByteLength() const JVMX_NOEXCEPT
{
  return Stream::GetByteLength();
}

const uint8_t * BigEndianStream::ToByteArray() const JVMX_NOEXCEPT
{
  return Stream::ToByteArray();
}

DataBuffer BigEndianStream::ToDataBuffer() const
{
  return Stream::ToDataBuffer();
}

uint8_t BigEndianStream::ReadUint8()
{
  return Stream::ReadUint8();
}

bool BigEndianStream::IsAtEnd() const JVMX_NOEXCEPT
{
  return Stream::IsAtEnd();
}

size_t BigEndianStream::GetBytesLeft() const JVMX_NOEXCEPT
{
  return Stream::GetBytesLeft();
}

std::string BigEndianStream::ToString() const
{
  std::stringstream result;
  result << "Pos: " << m_StreamPosition << " Length: " << m_Buffer.GetByteLength() << " Bytes: [";
  for ( size_t i = 0; i < m_Buffer.GetByteLength(); ++i )
  {
    result << (int)m_Buffer.ToByteArray()[ i ] << ",";
  }

  result << "]" << std::endl;

  return result.str();
}

// BigEndianStream &BigEndianStream::operator=(Stream other)
// {
//   Stream::swap( *this, other );
//   return *this;
// }

BigEndianStream &BigEndianStream::operator=(BigEndianStream other)
{
  Stream::swap( *this, other );
  return *this;
}


