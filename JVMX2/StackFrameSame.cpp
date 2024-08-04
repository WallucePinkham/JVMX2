#include "GlobalConstants.h"

#include "UnsupportedTypeException.h"
#include "DataBuffer.h"

#include "StackFrameSame.h"

extern const uint8_t c_MaxValidSameFrameTagNo = 63;

StackFrameSame::StackFrameSame( uint8_t tag ) :
  StackFrame( e_StackFrameTypeSameFrame ),
  m_OffsetDelta( tag )
{
  if ( tag > c_MaxValidSameFrameTagNo )
  {
    throw UnsupportedTypeException( __FUNCTION__ " - Tag was not int he valid range for StackFrameSame" );
  }
}

StackFrameSame::~StackFrameSame()
{}

uint16_t StackFrameSame::GetOffsetDelta() const JVMX_NOEXCEPT
{
  return m_OffsetDelta;
}

bool StackFrameSame::Equals( const StackFrame &other ) const
{
  if ( m_FrameType != other.GetType() )
  {
    return false;
  }

  return Equals( *reinterpret_cast<const StackFrameSame *>(&other) );
}

bool StackFrameSame::Equals( const StackFrameSame &other ) const
{
  return m_OffsetDelta == other.m_OffsetDelta;
}

DataBuffer StackFrameSame::ToBinary() const
{
  DataBuffer result = DataBuffer::EmptyBuffer();

  result = result.AppendUint8( m_OffsetDelta );

  return result;
}