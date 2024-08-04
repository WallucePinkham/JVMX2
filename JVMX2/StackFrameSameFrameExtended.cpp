
#include "Stream.h"
#include "StackFrameSameFrameExtended.h"

extern const uint8_t c_SameFrameExtendedTag = 63;

StackFrameSameFrameExtended::StackFrameSameFrameExtended( uint16_t offset ) :
StackFrame( e_StackFrameTypeSameFrameExtended )
, m_OffsetDelta( offset )
{
}

StackFrameSameFrameExtended::~StackFrameSameFrameExtended()
{
}

uint16_t StackFrameSameFrameExtended::GetOffsetDelta() const JVMX_NOEXCEPT
{
  return m_OffsetDelta;
}

StackFrameSameFrameExtended StackFrameSameFrameExtended::FromBinary( Stream &byteStream )
{
  return StackFrameSameFrameExtended( byteStream.ReadUint16() );
}

bool StackFrameSameFrameExtended::Equals( const StackFrame &other ) const
{
  if ( m_FrameType != other.GetType() )
  {
    return false;
  }

  return Equals( *reinterpret_cast<const StackFrameSameFrameExtended *>(&other) );
}

bool StackFrameSameFrameExtended::Equals( const StackFrameSameFrameExtended &other ) const
{
  return m_OffsetDelta == other.m_OffsetDelta;
}

DataBuffer StackFrameSameFrameExtended::ToBinary() const
{
  DataBuffer result = DataBuffer::EmptyBuffer();

  result = result.AppendUint8( c_SameFrameExtendedTag );
  result = result.AppendUint16( m_OffsetDelta );

  return result;
}
