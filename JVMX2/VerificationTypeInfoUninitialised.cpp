
#include "VerificationTypeInfoUninitialised.h"


VerificationTypeInfoUninitialised::VerificationTypeInfoUninitialised( uint16_t offset ) :
VerificationTypeInfo( e_VerificationTypeInfoTypes::Uninitialised )
, m_Offset( offset )
{
}

VerificationTypeInfoUninitialised::~VerificationTypeInfoUninitialised() JVMX_NOEXCEPT
{
}

uint16_t VerificationTypeInfoUninitialised::GetOffset() const JVMX_NOEXCEPT
{
  return m_Offset;
}

DataBuffer VerificationTypeInfoUninitialised::ToBinary() const
{
  DataBuffer buffer = VerificationTypeInfo::ToBinary();

  return buffer.AppendUint16( m_Offset );
}

bool VerificationTypeInfoUninitialised::Equals( const VerificationTypeInfo &other ) const
{
  if ( m_Type != other.GetType() )
  {
    return false;
  }

  return Equals( *reinterpret_cast<const VerificationTypeInfoUninitialised *>(&other) );
}

bool VerificationTypeInfoUninitialised::Equals( const VerificationTypeInfoUninitialised &other ) const
{
  return m_Offset == other.m_Offset;
}

