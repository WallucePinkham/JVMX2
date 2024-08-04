
#ifndef __VERIFICATIONTYPEINFOTOP_H__
#define __VERIFICATIONTYPEINFOTOP_H__

#include "DataBuffer.h"
#include "VerificationTypeInfo.h"

class VerificationTypeInfoTop : public VerificationTypeInfo
{
public:
  VerificationTypeInfoTop() : VerificationTypeInfo( e_VerificationTypeInfoTypes::Top ) {}
  virtual ~VerificationTypeInfoTop() JVMX_NOEXCEPT{};

  virtual DataBuffer ToBinary() const JVMX_OVERRIDE{ return VerificationTypeInfo::ToBinary(); }

  virtual bool Equals( const VerificationTypeInfo &other ) const JVMX_OVERRIDE{ return m_Type == other.GetType(); }
  virtual bool Equals( const VerificationTypeInfoTop & ) const { return true; }
};

#endif // __VERIFICATIONTYPEINFOTOP_H__
