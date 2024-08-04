
#ifndef __INVALIDSTATEEXCEPTION_H__
#define __INVALIDSTATEEXCEPTION_H__

#include "JVMXException.h"

class InvalidStateException : public JVMXException
{
public:
  InvalidStateException( const char *pMessage ) : JVMXException( pMessage )
  {}

  InvalidStateException( const char *pMessage, int errorCode ) : JVMXException( pMessage, errorCode )
  {}

  virtual ~InvalidStateException() JVMX_NOEXCEPT{};
};

#endif // __INVALIDSTATEEXCEPTION_H__

