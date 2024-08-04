
#ifndef __STACKFRAMEAPPENDFRAME_H__
#define __STACKFRAMEAPPENDFRAME_H__

#include <memory>

#include "StackFrame.h"
#include "VerificationTypeInfo.h"

class Stream; // Forward declaration

class StackFrameAppendFrame : public StackFrame
{
public:
  StackFrameAppendFrame( uint8_t tag, uint16_t offset, const VerificationTypeInfoList &verificationList );

  virtual ~StackFrameAppendFrame() JVMX_NOEXCEPT;

  virtual uint16_t GetOffsetDelta() const JVMX_NOEXCEPT JVMX_OVERRIDE;
  virtual uint8_t GetNumberOfAdditionalLocals() const JVMX_NOEXCEPT;

  virtual std::shared_ptr<VerificationTypeInfo> GetVerificationType( size_t index ) const;

  virtual bool Equals( const StackFrame &other ) const JVMX_OVERRIDE;
  virtual bool Equals( const StackFrameAppendFrame &other ) const;

  DataBuffer ToBinary() const JVMX_OVERRIDE;

public:
  static StackFrameAppendFrame FromBinary( uint8_t tag, Stream &byteStream );

private:
  uint8_t m_NumberOfAddionalLocals;
  uint16_t m_OffsetDelta;
  VerificationTypeInfoList m_VerificationInfoList;
};

#endif // __STACKFRAMEAPPENDFRAME_H__

