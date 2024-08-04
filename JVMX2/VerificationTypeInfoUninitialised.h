
#ifndef __VERIFICATIONTYPEINFOUNINITIALISED_H__
#define __VERIFICATIONTYPEINFOUNINITIALISED_H__

#include "DataBuffer.h"
#include "VerificationTypeInfo.h"

class VerificationTypeInfoUninitialised : public VerificationTypeInfo
{
public:
  VerificationTypeInfoUninitialised( uint16_t offset );
  virtual ~VerificationTypeInfoUninitialised() JVMX_NOEXCEPT;

  virtual uint16_t GetOffset() const JVMX_NOEXCEPT;
  virtual DataBuffer ToBinary() const JVMX_OVERRIDE;

  virtual bool Equals( const VerificationTypeInfo &other ) const JVMX_OVERRIDE;
  virtual bool Equals( const VerificationTypeInfoUninitialised &other ) const;

protected:
  uint16_t m_Offset;
};

#endif // __VERIFICATIONTYPEINFOUNINITIALISED_H__
