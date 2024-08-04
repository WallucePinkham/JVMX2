#include "UnsupportedTypeException.h"
#include "Stream.h"
#include "VerificationTypeInfoFactory.h"

#include "StackFrameSameLocals1StackItem.h"

extern const uint8_t c_MinValidLocals1StackItemTagNo = 64;
extern const uint8_t c_MaxValidLocals1StackItemTagNo = 127;

StackFrameSameLocals1StackItem::StackFrameSameLocals1StackItem( uint8_t tag, std::shared_ptr<VerificationTypeInfo> pVerification ) :
  StackFrame( e_StackFrameTypeSameLocals1StackItem ),
  m_VerificationInfo( pVerification )
{
  if ( tag < c_MinValidLocals1StackItemTagNo || tag > c_MaxValidLocals1StackItemTagNo )
  {
    throw UnsupportedTypeException( __FUNCTION__ " - Tag was not int he valid range for StackFrameSameLocals1StackItem" );
  }

  m_OffsetDelta = tag - c_MinValidLocals1StackItemTagNo;
}

StackFrameSameLocals1StackItem::StackFrameSameLocals1StackItem( const StackFrameSameLocals1StackItem &other ) :
  StackFrame( e_StackFrameTypeSameLocals1StackItem )
  , m_VerificationInfo( other.m_VerificationInfo )
  , m_OffsetDelta( other.m_OffsetDelta )
{}

StackFrameSameLocals1StackItem::~StackFrameSameLocals1StackItem()
{}

uint16_t StackFrameSameLocals1StackItem::GetOffsetDelta() const JVMX_NOEXCEPT
{
  return m_OffsetDelta;
}

StackFrameSameLocals1StackItem StackFrameSameLocals1StackItem::FromBinary( uint8_t tag, Stream &byteStream )
{
  VerificationTypeInfoFactory factory;

  uint8_t verificationTypeTag = byteStream.ReadUint8();

  return StackFrameSameLocals1StackItem( tag, factory.CreateVerificationTypeInfo( verificationTypeTag, byteStream ) );
}

bool StackFrameSameLocals1StackItem::Equals( const StackFrame &other ) const
{
  if ( m_FrameType != other.GetType() )
  {
    return false;
  }

  return Equals( *reinterpret_cast<const StackFrameSameLocals1StackItem *>(&other) );
}

bool StackFrameSameLocals1StackItem::Equals( const StackFrameSameLocals1StackItem &other ) const
{
  return m_OffsetDelta == other.m_OffsetDelta &&
    *m_VerificationInfo == *other.m_VerificationInfo;
}

DataBuffer StackFrameSameLocals1StackItem::ToBinary() const
{
  DataBuffer result = DataBuffer::EmptyBuffer();

  result = result.AppendUint8( c_MinValidLocals1StackItemTagNo + m_OffsetDelta );
  result = result.Append( m_VerificationInfo->ToBinary() );

  return result;
}

std::shared_ptr<VerificationTypeInfo> StackFrameSameLocals1StackItem::GetVerificationInfo() const JVMX_NOEXCEPT
{
  return m_VerificationInfo;
}