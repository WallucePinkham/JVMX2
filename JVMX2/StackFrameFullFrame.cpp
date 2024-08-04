
#include <algorithm>
#include <stdexcept>

#include "IndexOutOfBoundsException.h"

#include "Stream.h"
#include "VerificationTypeInfoFactory.h"

#include "StackFrameFullFrame.h"

extern const uint8_t c_StackFrameFullFrameTag = 255;

StackFrameFullFrame::StackFrameFullFrame( uint16_t offset, uint16_t numberOfLocals, const VerificationTypeInfoList &localsVerificationList, uint16_t numberOfStackItems, const VerificationTypeInfoList &stackVerificationList ) :
StackFrame( e_StackFrameTypeFull )
, m_OffsetDelta( offset )
, m_NumberOfLocals( numberOfLocals )
, m_NumberOfStackItems( numberOfStackItems )
, m_LocalsVerificationList( localsVerificationList )
, m_StackVerificationList( stackVerificationList )
{
}

StackFrameFullFrame::~StackFrameFullFrame() JVMX_NOEXCEPT
{
}

uint16_t StackFrameFullFrame::GetOffsetDelta() const JVMX_NOEXCEPT
{
  return m_OffsetDelta;
}

uint16_t StackFrameFullFrame::GetNumberOfLocals() const JVMX_NOEXCEPT
{
  return m_NumberOfLocals;
}

uint16_t StackFrameFullFrame::GetNumberOfStackItems() const JVMX_NOEXCEPT
{
  return m_NumberOfStackItems;
}

std::shared_ptr<VerificationTypeInfo> StackFrameFullFrame::GetLocalsVerificationType( size_t index ) const
{
  try
  {
    return m_LocalsVerificationList.at( index );
  }
  catch ( std::out_of_range & )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Index out of bounds for local verification type info in StackFrameFullFrame." );
  }
}

std::shared_ptr<VerificationTypeInfo> StackFrameFullFrame::GetStackVerificationType( size_t index ) const
{
  try
  {
    return m_StackVerificationList.at( index );
  }
  catch ( std::out_of_range & )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Index out of bounds for stack verification type info in StackFrameFullFrame." );
  }
}

StackFrameFullFrame StackFrameFullFrame::FromBinary( Stream &byteStream )
{
  uint16_t offset = byteStream.ReadUint16();
  uint16_t localsCount = byteStream.ReadUint16();

  VerificationTypeInfoFactory factory;

  VerificationTypeInfoList localsList = ReadVerificationTypeInfoList( localsCount, byteStream, factory );

  uint16_t stackItemCount = byteStream.ReadUint16();

  VerificationTypeInfoList stackItems = ReadVerificationTypeInfoList( stackItemCount, byteStream, factory );

  return StackFrameFullFrame( offset, localsCount, localsList, stackItemCount, stackItems );
}

VerificationTypeInfoList StackFrameFullFrame::ReadVerificationTypeInfoList( uint16_t localsCount, Stream &byteStream, VerificationTypeInfoFactory &factory )
{
  VerificationTypeInfoList infoList;
  for ( uint16_t i = 0; i < localsCount; ++ i )
  {
    uint8_t tag = byteStream.ReadUint8();

    infoList.push_back( factory.CreateVerificationTypeInfo( tag, byteStream ) );
  }

  return infoList;
}

bool StackFrameFullFrame::Equals( const StackFrame &other ) const
{
  if ( m_FrameType != other.GetType() )
  {
    return false;
  }

  return Equals( *reinterpret_cast<const StackFrameFullFrame *>(&other) );
}

bool StackFrameFullFrame::Equals( const StackFrameFullFrame &other ) const
{
  return m_OffsetDelta == other.m_OffsetDelta &&
    m_NumberOfLocals == other.m_NumberOfLocals &&
    m_NumberOfStackItems == other.m_NumberOfStackItems &&
    std::equal( m_LocalsVerificationList.cbegin(), m_LocalsVerificationList.cend(), other.m_LocalsVerificationList.cbegin(), []( const std::shared_ptr<VerificationTypeInfo> &left, const std::shared_ptr<VerificationTypeInfo> &right ) { return *left == *right; } ) &&
    std::equal( m_StackVerificationList.cbegin(), m_StackVerificationList.cend(), other.m_StackVerificationList.cbegin(), []( const std::shared_ptr<VerificationTypeInfo> &left, const std::shared_ptr<VerificationTypeInfo> &right ) { return *left == *right; } );
}

DataBuffer StackFrameFullFrame::ToBinary() const
{
  DataBuffer result = DataBuffer::EmptyBuffer();

  result = result.AppendUint8( c_StackFrameFullFrameTag );
  result = result.AppendUint16( m_OffsetDelta );

  result = result.AppendUint16( m_NumberOfLocals );
  for (std::shared_ptr<VerificationTypeInfo> pVerificationType : m_LocalsVerificationList)
  {
    result = result.Append( pVerificationType->ToBinary() );
  }

  result = result.AppendUint16( m_NumberOfStackItems );
  for (std::shared_ptr<VerificationTypeInfo> pVerificationType : m_StackVerificationList)
  {
    result = result.Append( pVerificationType->ToBinary() );
  }

  return result;
}
