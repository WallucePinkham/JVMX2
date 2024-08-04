
#ifndef __VERIFICATIONTYPEINFONULL_H__
#define __VERIFICATIONTYPEINFONULL_H__

#include "DataBuffer.h"
#include "VerificationTypeInfo.h"

class VerificationTypeInfoNull : public VerificationTypeInfo
{
public:
  VerificationTypeInfoNull() : VerificationTypeInfo( e_VerificationTypeInfoTypes::Null ) {}
  virtual ~VerificationTypeInfoNull() JVMX_NOEXCEPT{};

  virtual DataBuffer ToBinary() const JVMX_OVERRIDE{ return VerificationTypeInfo::ToBinary(); }

  virtual bool Equals( const VerificationTypeInfo &other ) const JVMX_OVERRIDE{ return m_Type == other.GetType(); }
  virtual bool Equals( const VerificationTypeInfoNull & ) const { return true; }
};

#endif // __VERIFICATIONTYPEINFONULL_H__
