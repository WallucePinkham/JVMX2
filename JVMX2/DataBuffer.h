
#ifndef __DATABUFFER_H__
#define __DATABUFFER_H__

#include <string>

#include "GlobalConstants.h"

#include "IJavaVariableType.h"
#include "IConstantPoolEntryValue.h"

class DataBuffer
{
public:
  static DataBuffer FromCString( const char16_t *pString );
  static DataBuffer FromByteArray( size_t length, const uint8_t *pBuffer );

protected:
  DataBuffer( size_t length, const uint8_t *pBuffer );

public:
  DataBuffer( const DataBuffer &other );
  DataBuffer( DataBuffer &&other ) JVMX_NOEXCEPT;

  virtual ~DataBuffer() JVMX_NOEXCEPT;

  DataBuffer &operator=(DataBuffer other);

  bool operator==(const DataBuffer &other) const;

  virtual const uint8_t *ToByteArray() const;

  virtual size_t GetByteLength() const;

  virtual DataBuffer Append( size_t length, const uint8_t *pBuffer ) const;

  virtual DataBuffer AppendUint8( uint8_t value ) const;
  virtual DataBuffer AppendUint16( uint16_t value ) const;
  virtual DataBuffer AppendUint32( uint32_t value ) const;
  virtual DataBuffer AppendUint64( uint64_t value ) const;
  virtual DataBuffer Append( const DataBuffer &other ) const;

  virtual bool IsEmpty() const;

public:
  static const DataBuffer EmptyBuffer();

  // In order to conform with the general C++ standards, this method name is intentionally all lower case.
  static void swap( DataBuffer &left, DataBuffer &right ) JVMX_NOEXCEPT;

private:
  void InternalCopyOtherValue( const uint8_t * pBuffer, size_t length );
  void InternalCleanup();

protected:
  friend class CodeSegmentDataBuffer;
  const uint8_t *GetRawDataPointer() const;

protected:
  uint8_t *m_pBytes;
  size_t m_Length;
};

#endif // __DATABUFFER_H__

