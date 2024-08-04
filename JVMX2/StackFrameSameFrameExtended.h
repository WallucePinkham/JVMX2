
#ifndef __STACKFRAMESAMEFRAMEEXTENDED_H__
#define __STACKFRAMESAMEFRAMEEXTENDED_H__

#include "StackFrame.h"

class Stream; // Forward declaration

class StackFrameSameFrameExtended : public StackFrame
{
public:
  StackFrameSameFrameExtended( uint16_t offset );

  virtual ~StackFrameSameFrameExtended();

  virtual uint16_t GetOffsetDelta() const JVMX_NOEXCEPT JVMX_OVERRIDE;

  virtual bool Equals( const StackFrame &other ) const JVMX_OVERRIDE;
  virtual bool Equals( const StackFrameSameFrameExtended &other ) const;

  virtual DataBuffer ToBinary() const JVMX_OVERRIDE;

public:
  static StackFrameSameFrameExtended FromBinary( Stream &byteStream );

private:
  uint16_t m_OffsetDelta;
};

#endif // __STACKFRAMESAMEFRAMEEXTENDED_H__
