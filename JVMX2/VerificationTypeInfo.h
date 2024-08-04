
#ifndef __VERIFICATIONTYPEINFO_H__
#define __VERIFICATIONTYPEINFO_H__

#include <vector>
#include <memory>

#include "GlobalConstants.h"

class DataBuffer; // Forward declaration

enum class e_VerificationTypeInfoTypes : uint8_t
{
  Top = 0,
  Integer = 1,
  Float = 2,
  Double = 3,
  Long = 4,
  Null = 5,
  UninitialisedThis = 6,
  Object = 7,
  Uninitialised = 8,
};

class VerificationTypeInfo JVMX_ABSTRACT
{
public:
  VerificationTypeInfo( e_VerificationTypeInfoTypes type );

  virtual ~VerificationTypeInfo() JVMX_NOEXCEPT;

  virtual e_VerificationTypeInfoTypes GetType() const;

  virtual DataBuffer ToBinary() const JVMX_ABSTRACT;

  bool operator==(const VerificationTypeInfo &other) const;
  virtual bool Equals( const VerificationTypeInfo &other ) const JVMX_PURE;

protected:
  e_VerificationTypeInfoTypes m_Type;
};

typedef std::vector< std::shared_ptr<VerificationTypeInfo> > VerificationTypeInfoList;

#endif // __VERIFICATIONTYPEINFO_H__
