
#ifndef __VERIFICATIONTYPEINFOCONSTANT_H__
#define __VERIFICATIONTYPEINFOCONSTANT_H__

#include "DataBuffer.h"
#include "VerificationTypeInfo.h"
#include "ConstantPoolClassReference.h"

class VerificationTypeInfoConstant : public VerificationTypeInfo
{
public:
  VerificationTypeInfoConstant( ConstantPoolIndex index );
  virtual ~VerificationTypeInfoConstant() JVMX_NOEXCEPT;

  virtual std::shared_ptr<ConstantPoolClassReference> GetClassReference() const JVMX_NOEXCEPT;
  virtual DataBuffer ToBinary() const JVMX_OVERRIDE;

  virtual bool Equals( const VerificationTypeInfo &other ) const JVMX_OVERRIDE;
  virtual bool Equals( const VerificationTypeInfoConstant &other ) const;

protected:
  std::shared_ptr<ConstantPoolClassReference> m_pClassReference;
};

#endif // __VERIFICATIONTYPEINFOCONSTANT_H__
