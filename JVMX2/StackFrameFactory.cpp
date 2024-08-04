
#include <memory>
#include "UnsupportedTypeException.h"

#include "StackFrameSame.h"
#include "StackFrameAppendFrame.h"
#include "StackFrameChop.h"
#include "StackFrameFullFrame.h"
#include "StackFrameSameFrameExtended.h"
#include "StackFrameSameLocals1StackItem.h"
#include "StackFrameSameLocals1StackItemFrameExtended.h"

#include "StackFrameFactory.h"

extern const uint8_t c_MaxValidSameFrameTagNo;
extern const uint8_t c_MaxValidLocals1StackItemTagNo;
const uint8_t c_SameLocals1StackItemExtendedTagNo = 247;
extern const uint8_t c_MinValidStackFrameChopTagNo;
extern const uint8_t c_MaxValidStackFrameChopTagNo;
const uint8_t c_SameFrameExtendedTagNo = 251;
extern const uint8_t c_MinValidAppendFrameTagNo;
extern const uint8_t c_MaxValidAppendFrameTagNo;
const uint8_t c_SameFrameFullFrame = 255;

StackFrameFactory::~StackFrameFactory()
{
}

std::shared_ptr<StackFrame> StackFrameFactory::CreateStackFrame( Stream &byteStream ) const
{
  uint8_t tag = byteStream.ReadUint8();

  if ( tag <= c_MaxValidSameFrameTagNo )
  {
    return std::make_shared<StackFrameSame>( tag );
  }

  if ( tag <= c_MaxValidLocals1StackItemTagNo )
  {
    return std::make_shared<StackFrameSameLocals1StackItem>( StackFrameSameLocals1StackItem::FromBinary( tag, byteStream ) );
  }

  if ( c_SameLocals1StackItemExtendedTagNo == tag )
  {
    return std::make_shared<StackFrameSameLocals1StackItemExtended>( StackFrameSameLocals1StackItemExtended::FromBinary( byteStream ) );
  }

  if ( tag >= c_MinValidStackFrameChopTagNo && tag <= c_MaxValidStackFrameChopTagNo )
  {
    return std::make_shared<StackFrameChop>( StackFrameChop::FromBinary( tag, byteStream ) );
  }

  if ( c_SameFrameExtendedTagNo == tag )
  {
    return std::make_shared<StackFrameSameFrameExtended>( StackFrameSameFrameExtended::FromBinary( byteStream ) );
  }

  if ( tag >= c_MinValidAppendFrameTagNo && tag <= c_MaxValidAppendFrameTagNo )
  {
    return std::make_shared<StackFrameAppendFrame>( StackFrameAppendFrame::FromBinary( tag, byteStream ) );
  }

  if ( c_SameFrameFullFrame == tag )
  {
    return std::make_shared<StackFrameFullFrame>( StackFrameFullFrame::FromBinary( byteStream ) );
  }

  throw UnsupportedTypeException( __FUNCTION__ " - Unknown stack frame type encountered." );
}

