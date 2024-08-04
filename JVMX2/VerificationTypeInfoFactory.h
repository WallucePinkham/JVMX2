
#ifndef __VERIFICATIONTYPEINFOFACTORY_H__
#define __VERIFICATIONTYPEINFOFACTORY_H__

#include <memory>
#include "VerificationTypeInfo.h"

class Stream; // Forward declaration.

class VerificationTypeInfoFactory
{
public:
  virtual ~VerificationTypeInfoFactory() JVMX_NOEXCEPT;

  std::shared_ptr<VerificationTypeInfo> CreateVerificationTypeInfo( uint8_t tag, Stream &byteStream );
};

#endif // __VERIFICATIONTYPEINFOFACTORY_H__
