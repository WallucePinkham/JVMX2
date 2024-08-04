
#ifndef __VERIFICATIONTYPEINFODOUBLE_H__
#define __VERIFICATIONTYPEINFODOUBLE_H__

#include "DataBuffer.h"
#include "VerificationTypeInfo.h"

class VerificationTypeInfoDouble : public VerificationTypeInfo
{
public:
  VerificationTypeInfoDouble() : VerificationTypeInfo( e_VerificationTypeInfoTypes::Double ) {}
  virtual ~VerificationTypeInfoDouble() JVMX_NOEXCEPT{};

  virtual DataBuffer ToBinary() const JVMX_OVERRIDE{ return VerificationTypeInfo::ToBinary(); }

  virtual bool Equals( const VerificationTypeInfo &other ) const JVMX_OVERRIDE{ return m_Type == other.GetType(); }
  virtual bool Equals( const VerificationTypeInfoDouble & ) const { return true; }
};


#endif // __VERIFICATIONTYPEINFODOUBLE_H__
