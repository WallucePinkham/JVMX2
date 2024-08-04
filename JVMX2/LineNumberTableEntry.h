
#ifndef _LINENUMBERTABLEENTRY__H_
#define _LINENUMBERTABLEENTRY__H_

#include <vector>

#include "GlobalConstants.h"
#include "ConstantPoolClassReference.h"

class LineNumberTableEntry
{
public:
  LineNumberTableEntry( uint16_t startPosition, uint16_t lineNumber );

  LineNumberTableEntry( const LineNumberTableEntry &other );

  virtual ~LineNumberTableEntry() JVMX_NOEXCEPT;

  LineNumberTableEntry &operator=(const LineNumberTableEntry &other);

  bool operator==(const LineNumberTableEntry &other) const JVMX_NOEXCEPT;

  virtual uint16_t GetStartPosition() const JVMX_NOEXCEPT;
  virtual uint16_t GetLineNumber() const JVMX_NOEXCEPT;

private:
  uint16_t m_StartPosition;
  uint16_t m_LineNumber;
};

typedef std::vector<LineNumberTableEntry> LineNumberTable;

#endif // _LINENUMBERTABLEENTRY__H_

