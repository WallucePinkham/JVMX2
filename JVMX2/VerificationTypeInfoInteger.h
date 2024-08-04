
#ifndef __VERIFICATIONTYPEINFOINTEGER_H__
#define __VERIFICATIONTYPEINFOINTEGER_H__

#include "DataBuffer.h"
#include "VerificationTypeInfo.h"

class VerificationTypeInfoInteger : public VerificationTypeInfo
{
public:
  VerificationTypeInfoInteger() : VerificationTypeInfo( e_VerificationTypeInfoTypes::Integer ) {}
  virtual ~VerificationTypeInfoInteger() JVMX_NOEXCEPT{};

  virtual DataBuffer ToBinary() const JVMX_OVERRIDE{ return VerificationTypeInfo::ToBinary(); }

  virtual bool Equals( const VerificationTypeInfo &other ) const JVMX_OVERRIDE{ return m_Type == other.GetType(); }
  virtual bool Equals( const VerificationTypeInfoInteger & ) const { return true; }
};

#endif // __VERIFICATIONTYPEINFOINTEGER_H__
