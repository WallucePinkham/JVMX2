
#include "StackFrame.h"

#include "TypeMismatchException.h"
#include "StackFrameSameLocals1StackItem.h"

StackFrame::StackFrame( e_StackFrameTypes type ) :
m_FrameType( type )
{}

e_StackFrameTypes StackFrame::GetType() const JVMX_NOEXCEPT
{
  return m_FrameType;
}

StackFrame::~StackFrame() JVMX_NOEXCEPT
{
}

StackFrameSameLocals1StackItem StackFrame::ToStackFrameSameLocals1StackItem() const
{
  if ( m_FrameType != e_StackFrameTypeSameLocals1StackItem )
  {
    throw TypeMismatchException( __FUNCTION__ " - Trying to convert to StackFrameSameLocals1StackItem when the type is not a StackFrameSameLocals1StackItem." );
  }

  const StackFrameSameLocals1StackItem *pResult = dynamic_cast<const StackFrameSameLocals1StackItem *>(this);
  if ( nullptr == pResult )
  {
    throw TypeMismatchException( __FUNCTION__ " - Dynamic cast to StackFrameSameLocals1StackItem failed." );
  }

  return StackFrameSameLocals1StackItem( *pResult );
}

bool StackFrame::operator==(const StackFrame &other) const
{
  if ( m_FrameType != other.m_FrameType )
  {
    return false;
  }

  return Equals( other );
}
