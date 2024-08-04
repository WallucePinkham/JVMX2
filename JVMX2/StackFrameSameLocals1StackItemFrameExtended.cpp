
#include "Stream.h"
#include "VerificationTypeInfoFactory.h"
#include "StackFrameSameLocals1StackItemFrameExtended.h"

const uint8_t c_StackFrameSameLocals1StackItemExtendedTag = 247;

StackFrameSameLocals1StackItemExtended::StackFrameSameLocals1StackItemExtended( uint16_t offsetDelta, std::shared_ptr<VerificationTypeInfo> pVerification ) :
StackFrame( e_StackFrameTypeSameLocals1StackFrameExtended )
, m_OffsetDelta( offsetDelta )
, m_VerificationInfo( pVerification )
{
}

StackFrameSameLocals1StackItemExtended::~StackFrameSameLocals1StackItemExtended()
{
}

uint16_t StackFrameSameLocals1StackItemExtended::GetOffsetDelta() const JVMX_NOEXCEPT
{
  return m_OffsetDelta;
}

StackFrameSameLocals1StackItemExtended StackFrameSameLocals1StackItemExtended::FromBinary( Stream &byteStream )
{
  uint16_t offSet = byteStream.ReadUint16();
  VerificationTypeInfoFactory factory;
  uint8_t tag = byteStream.ReadUint8();

  return StackFrameSameLocals1StackItemExtended( offSet, factory.CreateVerificationTypeInfo( tag, byteStream ) );
}

bool StackFrameSameLocals1StackItemExtended::Equals( const StackFrame &other ) const
{
  if ( m_FrameType != other.GetType() )
  {
    return false;
  }

  return Equals( *reinterpret_cast<const StackFrameSameLocals1StackItemExtended *>(&other) );
}

bool StackFrameSameLocals1StackItemExtended::Equals( const StackFrameSameLocals1StackItemExtended &other ) const
{
  return m_OffsetDelta == other.m_OffsetDelta &&
    *m_VerificationInfo == *other.m_VerificationInfo;
}

DataBuffer StackFrameSameLocals1StackItemExtended::ToBinary() const
{
  DataBuffer result = DataBuffer::EmptyBuffer();

  result = result.AppendUint8( c_StackFrameSameLocals1StackItemExtendedTag );
  result = result.AppendUint16( m_OffsetDelta );
  result = result.Append( m_VerificationInfo->ToBinary() );

  return result;
}

const std::shared_ptr<VerificationTypeInfo> StackFrameSameLocals1StackItemExtended::GetVerificationInfo() const JVMX_NOEXCEPT
{
  return m_VerificationInfo;
}
