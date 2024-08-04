
#ifndef __VERIFICATIONTYPEINFOLONG_H__
#define __VERIFICATIONTYPEINFOLONG_H__

#include "DataBuffer.h"
#include "VerificationTypeInfo.h"

class VerificationTypeInfoLong : public VerificationTypeInfo
{
public:
  VerificationTypeInfoLong() : VerificationTypeInfo( e_VerificationTypeInfoTypes::Long ) {}
  virtual ~VerificationTypeInfoLong() JVMX_NOEXCEPT{};

  virtual DataBuffer ToBinary() const JVMX_OVERRIDE{ return VerificationTypeInfo::ToBinary(); }

  virtual bool Equals( const VerificationTypeInfo &other ) const JVMX_OVERRIDE{ return m_Type == other.GetType(); }
  virtual bool Equals( const VerificationTypeInfoLong & ) const { return true; }
};

#endif // __VERIFICATIONTYPEINFOLONG_H__
