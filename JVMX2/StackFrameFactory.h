
#ifndef __STACKFRAMEFACTORY_H__
#define __STACKFRAMEFACTORY_H__

#include "Stream.h"
#include "StackFrame.h"
#include "GlobalConstants.h"

class StackFrameFactory
{
public:
  virtual ~StackFrameFactory();

  virtual std::shared_ptr<StackFrame> CreateStackFrame( Stream &byteStream ) const;

private:
};

#endif // __STACKFRAMEFACTORY_H__
