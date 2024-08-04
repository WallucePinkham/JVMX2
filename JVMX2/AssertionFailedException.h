
#ifndef __ASSERTIONFAILEDEXCEPTION_H__
#define __ASSERTIONFAILEDEXCEPTION_H__

#include "JVMXException.h"

class AssertionFailedException : public JVMXException
{
public:
  AssertionFailedException( const char *pMessage ) : JVMXException( pMessage )
  {}

  AssertionFailedException( const char *pMessage, int errorCode ) : JVMXException( pMessage, errorCode )
  {}

  virtual ~AssertionFailedException() JVMX_NOEXCEPT {};
};

#endif // __ASSERTIONFAILEDEXCEPTION_H__

