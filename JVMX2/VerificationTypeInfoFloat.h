
#ifndef __VERIFICATIONTYPEINFOFLOAT_H__
#define __VERIFICATIONTYPEINFOFLOAT_H__

#include "DataBuffer.h"
#include "VerificationTypeInfo.h"

class VerificationTypeInfoFloat : public VerificationTypeInfo
{
public:
  VerificationTypeInfoFloat() : VerificationTypeInfo( e_VerificationTypeInfoTypes::Float ) {}
  virtual ~VerificationTypeInfoFloat() JVMX_NOEXCEPT{};

  virtual DataBuffer ToBinary() const JVMX_OVERRIDE{ return VerificationTypeInfo::ToBinary(); }

  virtual bool Equals( const VerificationTypeInfo &other ) const JVMX_OVERRIDE{ return m_Type == other.GetType(); }
  virtual bool Equals( const VerificationTypeInfoFloat & ) const { return true; }
};

#endif // __VERIFICATIONTYPEINFOFLOAT_H__
