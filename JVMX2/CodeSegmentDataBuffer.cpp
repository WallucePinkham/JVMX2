
#include "CodeSegmentDataBuffer.h"

CodeSegmentDataBuffer::CodeSegmentDataBuffer( const DataBuffer &dataBuffer )
  : m_Data( dataBuffer )
{
}

const uint8_t *CodeSegmentDataBuffer::GetRawDataPointer() const
{
  return m_Data.GetRawDataPointer();
}

size_t CodeSegmentDataBuffer::GetByteLength() const
{
  return m_Data.GetByteLength();
}

