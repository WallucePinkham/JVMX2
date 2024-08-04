
#ifndef __STACKFRAMESAME_H__
#define __STACKFRAMESAME_H__

#include "StackFrame.h"

class DataBuffer; // Forward declaration

extern const uint8_t c_MaxValidSameFrameTagNo;

class StackFrameSame : public StackFrame
{
public:
  StackFrameSame( uint8_t tag );

  virtual ~StackFrameSame();

  virtual uint16_t GetOffsetDelta() const JVMX_NOEXCEPT JVMX_OVERRIDE;

  virtual bool Equals( const StackFrame &other ) const JVMX_OVERRIDE;
  virtual bool Equals( const StackFrameSame &other ) const;

  virtual DataBuffer ToBinary() const JVMX_OVERRIDE;

private:
  uint8_t m_OffsetDelta;
};

#endif // __STACKFRAMESAME_H__
