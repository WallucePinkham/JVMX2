
#include "DataBuffer.h"
#include "VerificationTypeInfo.h"

VerificationTypeInfo::VerificationTypeInfo( e_VerificationTypeInfoTypes type ) : m_Type( type )
{
}

VerificationTypeInfo::~VerificationTypeInfo() JVMX_NOEXCEPT
{
}

e_VerificationTypeInfoTypes VerificationTypeInfo::GetType() const
{
  return m_Type;
}


bool VerificationTypeInfo::operator==(const VerificationTypeInfo &other) const
{
  if ( m_Type != other.m_Type )
  {
    return false;
  }

  return Equals( other );
}

DataBuffer VerificationTypeInfo::ToBinary() const
{
  DataBuffer result = DataBuffer::EmptyBuffer();

  return result.AppendUint8( static_cast<uint8_t>(m_Type) );
}
