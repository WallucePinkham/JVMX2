#include <stdexcept>

#include "InvalidArgumentException.h"
#include "UnsupportedTypeException.h"
#include "IndexOutOfBoundsException.h"

#include "Stream.h"
#include "VerificationTypeInfoFactory.h"

#include "StackFrameAppendFrame.h"

extern const uint8_t c_MinValidAppendFrameTagNo = 252;
extern const uint8_t c_MaxValidAppendFrameTagNo = 254;

const uint8_t c_LastAdditionalLocalsOffset = 251;

StackFrameAppendFrame::StackFrameAppendFrame( uint8_t tag, uint16_t offset, const VerificationTypeInfoList &verificationList ) :
  StackFrame( e_StackFrameTypeAppend )
  , m_OffsetDelta( offset )
  , m_VerificationInfoList( verificationList )
{
  if ( tag < c_MinValidAppendFrameTagNo || tag > c_MaxValidAppendFrameTagNo )
  {
    throw UnsupportedTypeException( __FUNCTION__ " - Tag was not int he valid range for StackFrameAppendFrame." );
  }

  m_NumberOfAddionalLocals = tag - c_LastAdditionalLocalsOffset;

  if ( m_NumberOfAddionalLocals != verificationList.size() )
  {
    throw InvalidArgumentException( __FUNCTION__ " - The number of additional local variables didn't match the number of items in the list of VerificationTypeInfo structures." );
  }
}

StackFrameAppendFrame::~StackFrameAppendFrame() JVMX_NOEXCEPT
{}

uint16_t StackFrameAppendFrame::GetOffsetDelta() const JVMX_NOEXCEPT
{
  return m_OffsetDelta;
}

uint8_t StackFrameAppendFrame::GetNumberOfAdditionalLocals() const JVMX_NOEXCEPT
{
  return m_NumberOfAddionalLocals;
}

std::shared_ptr<VerificationTypeInfo> StackFrameAppendFrame::GetVerificationType( size_t index ) const
{
  try
  {
    return m_VerificationInfoList.at( index );
  }
  catch ( std::out_of_range & )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Index out of bounds for verification type info in StackFrameAppendFrame." );
  }
}

StackFrameAppendFrame StackFrameAppendFrame::FromBinary( uint8_t tag, Stream &byteStream )
{
  uint16_t offset = byteStream.ReadUint16();
  uint8_t numberOfAddionalLocals = tag - c_LastAdditionalLocalsOffset;

  VerificationTypeInfoList list;
  VerificationTypeInfoFactory factory;

  for ( uint8_t i = 0; i < numberOfAddionalLocals; ++ i )
  {
    uint8_t verificationInfoTypeTag = byteStream.ReadUint8();

    list.push_back( factory.CreateVerificationTypeInfo( verificationInfoTypeTag, byteStream ) );
  }

  return StackFrameAppendFrame( tag, offset, list );
}

bool StackFrameAppendFrame::Equals( const StackFrame &other ) const
{
  if ( m_FrameType != other.GetType() )
  {
    return false;
  }

  return Equals( *reinterpret_cast<const StackFrameAppendFrame *>(&other) );
}

bool StackFrameAppendFrame::Equals( const StackFrameAppendFrame &other ) const
{
  return m_NumberOfAddionalLocals == other.m_NumberOfAddionalLocals &&
    m_OffsetDelta == other.m_OffsetDelta &&
    std::equal( m_VerificationInfoList.cbegin(), m_VerificationInfoList.cend(), other.m_VerificationInfoList.cbegin(), []( const std::shared_ptr<VerificationTypeInfo> &left, const std::shared_ptr<VerificationTypeInfo> &right ) { return *left == *right; } );
}

DataBuffer StackFrameAppendFrame::ToBinary() const
{
  DataBuffer result = DataBuffer::EmptyBuffer();

  result = result.AppendUint8( c_LastAdditionalLocalsOffset + m_NumberOfAddionalLocals );
  result = result.AppendUint16( m_OffsetDelta );

  for (std::shared_ptr<VerificationTypeInfo> pVerificationInfo : m_VerificationInfoList)
  {
    result = result.Append( pVerificationInfo->ToBinary() );
  }

  return result;
}