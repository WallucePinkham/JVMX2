
#ifndef __STACKFRAMESAMELOCALS1STACKITEMFRAMEEXTENDED_H__
#define __STACKFRAMESAMELOCALS1STACKITEMFRAMEEXTENDED_H__

#include <memory>

#include "StackFrame.h"
#include "VerificationTypeInfo.h"

class Stream; // Forward declaration;

class StackFrameSameLocals1StackItemExtended : public StackFrame
{
public:
  StackFrameSameLocals1StackItemExtended( uint16_t offsetDelta, std::shared_ptr<VerificationTypeInfo> pVerification );

  virtual ~StackFrameSameLocals1StackItemExtended();

  virtual uint16_t GetOffsetDelta() const JVMX_NOEXCEPT JVMX_OVERRIDE;
  virtual const std::shared_ptr<VerificationTypeInfo> GetVerificationInfo() const JVMX_NOEXCEPT;

  static StackFrameSameLocals1StackItemExtended FromBinary( Stream &byteStream );

  virtual bool Equals( const StackFrame &other ) const JVMX_OVERRIDE;
  virtual bool Equals( const StackFrameSameLocals1StackItemExtended &other ) const;

  virtual DataBuffer ToBinary() const JVMX_OVERRIDE;

private:
  uint16_t m_OffsetDelta;
  std::shared_ptr<VerificationTypeInfo> m_VerificationInfo;
};

#endif // __STACKFRAMESAMELOCALS1STACKITEMFRAMEEXTENDED_H__
