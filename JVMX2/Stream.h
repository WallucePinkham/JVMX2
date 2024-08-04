
#ifndef __STREAM_H__
#define __STREAM_H__

#include "GlobalConstants.h"
#include "DataBuffer.h"
#include "IndexOutOfBoundsException.h"

class Stream
{
private:
  explicit Stream( const DataBuffer &buffer );

public:
  //Stream( Stream &&other ) JVMX_NOEXCEPT;
  Stream( const Stream &other );

  Stream &operator=(Stream other);

  static Stream FromByteArray( size_t length, const uint8_t *pBuffer );
  static Stream FromDataBuffer( const DataBuffer &buffer );

  virtual ~Stream() JVMX_NOEXCEPT;

  virtual size_t GetByteLength() const JVMX_NOEXCEPT;

  // This function starts at the beginning of the stream (not taking position into account) and puts what is left in a databuffer
  virtual const uint8_t *ToByteArray() const JVMX_NOEXCEPT;

  // This function starts at the current position and puts what is left in a databuffer
  virtual DataBuffer ToDataBuffer() const;

  virtual uint8_t ReadUint8();
  virtual uint16_t ReadUint16();
  virtual uint32_t ReadUint32();
  virtual uint64_t ReadUint64();

  virtual DataBuffer ReadBytes( size_t length );

  virtual bool IsAtEnd() const JVMX_NOEXCEPT;
  virtual size_t GetBytesLeft() const JVMX_NOEXCEPT;

public:
  // In order to conform with the general C++ standards, this method name is intentionally all lower case.
  static void swap( Stream &left, Stream &right ) JVMX_NOEXCEPT;

  static Stream EmptyStream();

protected:
  DataBuffer m_Buffer;
  size_t m_StreamPosition;

protected:
  template<typename T> T ReadFromBuffer();
};

template<typename T>
inline T Stream::ReadFromBuffer()
{
  if ( m_StreamPosition + sizeof( T ) > m_Buffer.GetByteLength() )
  {
    throw IndexOutOfBoundsException( __FUNCSIG__ " - Trying to read past the end of the buffer." );
  }

  const uint8_t *pBuffer = m_Buffer.ToByteArray();
  T result = *(reinterpret_cast<const T *>(pBuffer + m_StreamPosition));

  m_StreamPosition += sizeof( T );

  return result;
}

#endif // __STREAM_H__
