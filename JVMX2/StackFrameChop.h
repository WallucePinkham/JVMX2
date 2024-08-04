
#ifndef __STACKFRAMECHOP_H__
#define __STACKFRAMECHOP_H__

#include "StackFrame.h"

class Stream; // Forward declaration

class StackFrameChop : public StackFrame
{
public:
  StackFrameChop( uint8_t tag, uint16_t offset );

  virtual ~StackFrameChop();

  virtual uint16_t GetOffsetDelta() const JVMX_NOEXCEPT JVMX_OVERRIDE;
  virtual uint8_t GetNumberOfLastAbsentLocals() const JVMX_NOEXCEPT;

  virtual bool Equals( const StackFrame &other ) const JVMX_OVERRIDE;
  virtual bool Equals( const StackFrameChop &other ) const;

  DataBuffer ToBinary() const JVMX_OVERRIDE;

public:
  static StackFrameChop FromBinary( uint8_t tag, Stream &byteStream );

private:
  uint8_t m_NumberOfLastAbsentLocals;
  uint16_t m_OffsetDelta;
};

#endif // __STACKFRAMECHOP_H__
