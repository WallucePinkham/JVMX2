
#ifndef __STACKFRAMESAMELOCALS1STACKITEM_H__
#define __STACKFRAMESAMELOCALS1STACKITEM_H__

#include <memory>

#include "StackFrame.h"
#include "VerificationTypeInfo.h"

class Stream; // Forward declaration

class StackFrameSameLocals1StackItem : public StackFrame
{
public:
  StackFrameSameLocals1StackItem( uint8_t tag, std::shared_ptr<VerificationTypeInfo> pVerification );

  StackFrameSameLocals1StackItem( const StackFrameSameLocals1StackItem &other );

  virtual ~StackFrameSameLocals1StackItem();

  virtual uint16_t GetOffsetDelta() const JVMX_NOEXCEPT JVMX_OVERRIDE;
  virtual std::shared_ptr<VerificationTypeInfo> GetVerificationInfo() const JVMX_NOEXCEPT;

  static StackFrameSameLocals1StackItem FromBinary( uint8_t tag, Stream &byteStream );

  virtual DataBuffer ToBinary() const JVMX_OVERRIDE;

  virtual bool Equals( const StackFrame &other ) const JVMX_OVERRIDE;
  virtual bool Equals( const StackFrameSameLocals1StackItem &other ) const;

private:
  uint8_t m_OffsetDelta;
  std::shared_ptr<VerificationTypeInfo> m_VerificationInfo;
};

#endif // __STACKFRAMESAMELOCALS1STACKITEM_H__
