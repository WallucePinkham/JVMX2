
#include "LineNumberTableEntry.h"

LineNumberTableEntry::LineNumberTableEntry( uint16_t startPosition, uint16_t lineNumber )
  : m_StartPosition( startPosition )
  , m_LineNumber( lineNumber )
{
}

LineNumberTableEntry::LineNumberTableEntry( const LineNumberTableEntry &other )
{
  *this = other;
}

LineNumberTableEntry::~LineNumberTableEntry() JVMX_NOEXCEPT
{
}

LineNumberTableEntry &LineNumberTableEntry::operator=(const LineNumberTableEntry &other)
{
  if ( this != &other )
  {
    m_StartPosition = other.m_StartPosition;
    m_LineNumber = other.m_LineNumber;
  }

  return *this;
}


uint16_t LineNumberTableEntry::GetStartPosition() const JVMX_NOEXCEPT
{
  return m_StartPosition;
}

uint16_t LineNumberTableEntry::GetLineNumber() const JVMX_NOEXCEPT
{
  return m_LineNumber;
}

bool LineNumberTableEntry::operator==(const LineNumberTableEntry &other) const JVMX_NOEXCEPT
{
  return m_LineNumber == other.m_LineNumber &&
  m_StartPosition == other.m_StartPosition;
}

