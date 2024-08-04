
#ifndef __STACKMAPFRAME_H__
#define __STACKMAPFRAME_H__

#include <memory>
#include <vector>

#include "GlobalConstants.h"

class StackFrameSameLocals1StackItem;

enum e_StackFrameTypes : uint8_t
{
  e_StackFrameTypeSameFrame,
  e_StackFrameTypeSameLocals1StackItem,
  e_StackFrameTypeSameLocals1StackFrameExtended,
  e_StackFrameTypeChop,
  e_StackFrameTypeSameFrameExtended,
  e_StackFrameTypeAppend,
  e_StackFrameTypeFull,
};

class DataBuffer; // Forward declaration

class StackFrame JVMX_ABSTRACT
{
protected:
  StackFrame( e_StackFrameTypes type );

public:
  virtual e_StackFrameTypes GetType() const JVMX_NOEXCEPT;
  virtual uint16_t GetOffsetDelta() const JVMX_NOEXCEPT JVMX_PURE;

  virtual ~StackFrame() JVMX_NOEXCEPT;

  StackFrameSameLocals1StackItem ToStackFrameSameLocals1StackItem() const;

  bool operator==(const StackFrame &other) const;
  virtual bool Equals( const StackFrame &other ) const JVMX_PURE;

  virtual DataBuffer ToBinary() const JVMX_PURE;

protected:
  e_StackFrameTypes m_FrameType;
};

typedef std::vector<std::shared_ptr<StackFrame> > StackFrameList;

#endif // __STACKMAPFRAME_H__
