#include "ExceptionTableEntry.h"

ExceptionTableEntry::ExceptionTableEntry( uint16_t startPosition, uint16_t endPosition, uint16_t handlerPosition, std::shared_ptr<ConstantPoolClassReference> pCatchType )
  : m_pCatchType( pCatchType )
  , m_EndPosition( endPosition )
  , m_HandlerPosition( handlerPosition )
  , m_StartPosition( startPosition )
{
}

ExceptionTableEntry::ExceptionTableEntry( const ExceptionTableEntry &other ) : m_pCatchType( other.m_pCatchType )
{
  *this = other;
}

ExceptionTableEntry::~ExceptionTableEntry()
{
}

ExceptionTableEntry &ExceptionTableEntry::operator=(const ExceptionTableEntry &other)
{
  if ( this != &other )
  {
    m_pCatchType = other.m_pCatchType;
    m_EndPosition = other.m_EndPosition;
    m_StartPosition = other.m_StartPosition;
    m_HandlerPosition = other.m_HandlerPosition;
  }

  return *this;
}


std::shared_ptr<ConstantPoolClassReference> ExceptionTableEntry::GetCatchType() const
{
  return m_pCatchType;
}

uint16_t ExceptionTableEntry::GetHandlerPosition() const
{
  return m_HandlerPosition;
}

uint16_t ExceptionTableEntry::GetEndPosition() const
{
  return m_EndPosition;
}

uint16_t ExceptionTableEntry::GetStartPosition() const
{
  return m_StartPosition;
}

bool ExceptionTableEntry::operator==(const ExceptionTableEntry &other) const JVMX_NOEXCEPT
{
  return *m_pCatchType == *other.m_pCatchType &&
  m_EndPosition == other.m_EndPosition &&
  m_StartPosition == other.m_StartPosition &&
  m_HandlerPosition == other.m_HandlerPosition;
}

