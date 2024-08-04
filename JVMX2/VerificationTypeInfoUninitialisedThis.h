
#ifndef __VERIFICATIONTYPEINFOUNINITIALISEDTHIS_H__
#define __VERIFICATIONTYPEINFOUNINITIALISEDTHIS_H__

#include "DataBuffer.h"
#include "VerificationTypeInfo.h"

class VerificationTypeInfoUninitialsedThis : public VerificationTypeInfo
{
public:
  VerificationTypeInfoUninitialsedThis() : VerificationTypeInfo( e_VerificationTypeInfoTypes::UninitialisedThis ) {}
  virtual ~VerificationTypeInfoUninitialsedThis() JVMX_NOEXCEPT{};

  virtual DataBuffer ToBinary() const JVMX_OVERRIDE{ return VerificationTypeInfo::ToBinary(); }

  virtual bool Equals( const VerificationTypeInfo &other ) const JVMX_OVERRIDE{ return m_Type == other.GetType(); }
  virtual bool Equals( const VerificationTypeInfoUninitialsedThis & ) const { return true; }
};

#endif // __VERIFICATIONTYPEINFOUNINITIALISEDTHIS_H__
