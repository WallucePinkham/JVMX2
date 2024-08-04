
#ifndef _CODESEGMENTDATABUFFER__H_
#define _CODESEGMENTDATABUFFER__H_

#include "DataBuffer.h"

class CodeSegmentDataBuffer
{
public:
  CodeSegmentDataBuffer( const DataBuffer &dataBuffer );

  const uint8_t *GetRawDataPointer() const;
  size_t GetByteLength() const;

private:
  CodeSegmentDataBuffer operator=(const CodeSegmentDataBuffer &other);

private:
  const DataBuffer &m_Data;
};

#endif // _CODESEGMENTDATABUFFER__H_
