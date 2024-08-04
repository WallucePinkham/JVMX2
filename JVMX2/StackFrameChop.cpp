#include "UnsupportedTypeException.h"

#include "Stream.h"

#include "StackFrameChop.h"

extern const uint8_t c_MinValidStackFrameChopTagNo = 248;
extern const uint8_t c_MaxValidStackFrameChopTagNo = 250;

const uint8_t c_LastLocalsAbsentOffset = 251;

StackFrameChop::StackFrameChop( uint8_t tag, uint16_t offset )
  : StackFrame( e_StackFrameTypeChop )
  , m_OffsetDelta( offset )
{
  if ( tag < c_MinValidStackFrameChopTagNo || tag > c_MaxValidStackFrameChopTagNo )
  {
    throw UnsupportedTypeException( __FUNCTION__ " - Tag was not int he valid range for StackFrameChop" );
  }

  m_NumberOfLastAbsentLocals = c_LastLocalsAbsentOffset - tag;
}

StackFrameChop::~StackFrameChop()
{}

uint16_t StackFrameChop::GetOffsetDelta() const JVMX_NOEXCEPT
{
  return m_OffsetDelta;
}

uint8_t StackFrameChop::GetNumberOfLastAbsentLocals() const JVMX_NOEXCEPT
{
  return m_NumberOfLastAbsentLocals;
}

StackFrameChop StackFrameChop::FromBinary( uint8_t tag, Stream &byteStream )
{
  uint16_t offsetDelta = byteStream.ReadUint16();

  return StackFrameChop( tag, offsetDelta );
}

bool StackFrameChop::Equals( const StackFrame &other ) const
{
  if ( m_FrameType != other.GetType() )
  {
    return false;
  }

  return Equals( *reinterpret_cast<const StackFrameChop *>(&other) );
}

bool StackFrameChop::Equals( const StackFrameChop &other ) const
{
  return m_NumberOfLastAbsentLocals == other.m_NumberOfLastAbsentLocals &&
    m_OffsetDelta == other.m_OffsetDelta;
}

DataBuffer StackFrameChop::ToBinary() const
{
  DataBuffer result = DataBuffer::EmptyBuffer();

  result = result.AppendUint8( c_LastLocalsAbsentOffset - m_NumberOfLastAbsentLocals );
  result = result.AppendUint16( m_OffsetDelta );

  return result;
}