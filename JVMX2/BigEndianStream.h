
#ifndef _BIGENDIANSTREAM__H_
#define _BIGENDIANSTREAM__H_

#include <string>

#include "Stream.h"

class BigEndianStream : public Stream
{
public:
  BigEndianStream( const Stream &contained );
  //BigEndianStream( Stream &&contained );

  virtual ~BigEndianStream() JVMX_NOEXCEPT {};

  //BigEndianStream &operator=(Stream other);
  BigEndianStream &operator=(BigEndianStream other);

  virtual uint16_t ReadUint16() JVMX_OVERRIDE;

  virtual uint32_t ReadUint32() JVMX_OVERRIDE;

  virtual uint64_t ReadUint64() JVMX_OVERRIDE;

  virtual DataBuffer ReadBytes( size_t length ) JVMX_OVERRIDE;

  virtual size_t GetByteLength() const JVMX_NOEXCEPT JVMX_OVERRIDE;

  virtual const uint8_t * ToByteArray() const JVMX_NOEXCEPT JVMX_OVERRIDE;

  virtual DataBuffer ToDataBuffer() const JVMX_OVERRIDE;

  virtual uint8_t ReadUint8() JVMX_OVERRIDE;

  virtual bool IsAtEnd() const JVMX_NOEXCEPT JVMX_OVERRIDE;

  virtual size_t GetBytesLeft() const JVMX_NOEXCEPT JVMX_OVERRIDE;

  virtual std::string ToString() const;
};

#endif // _BIGENDIANSTREAM__H_
