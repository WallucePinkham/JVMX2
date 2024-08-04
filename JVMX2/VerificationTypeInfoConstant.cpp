
#include "VerificationTypeInfoConstant.h"

VerificationTypeInfoConstant::VerificationTypeInfoConstant( ConstantPoolIndex index ) :
  VerificationTypeInfo( e_VerificationTypeInfoTypes::Object )
  , m_pClassReference( ConstantPoolClassReference::FromConstantPoolIndex( index ) )
{
}

VerificationTypeInfoConstant::~VerificationTypeInfoConstant() JVMX_NOEXCEPT
{
}

std::shared_ptr<ConstantPoolClassReference> VerificationTypeInfoConstant::GetClassReference() const JVMX_NOEXCEPT
{
  return m_pClassReference;
}

DataBuffer VerificationTypeInfoConstant::ToBinary() const
{
  DataBuffer buffer = VerificationTypeInfo::ToBinary();

  return buffer.AppendUint16( static_cast<uint16_t>(m_pClassReference->ToConstantPoolIndex()) );
}

bool VerificationTypeInfoConstant::Equals( const VerificationTypeInfo &other ) const
{
  if ( m_Type != other.GetType() )
  {
    return false;
  }

  return Equals( *reinterpret_cast<const VerificationTypeInfoConstant*>(&other) );
}

bool VerificationTypeInfoConstant::Equals( const VerificationTypeInfoConstant &other ) const
{
  return *m_pClassReference == *other.m_pClassReference;
}

