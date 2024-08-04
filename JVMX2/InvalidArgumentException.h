
#ifndef __INVALIDARGUMENTEXCEPTION_H__
#define __INVALIDARGUMENTEXCEPTION_H__

#include "JVMXException.h"

class InvalidArgumentException : public JVMXException
{
public:
  InvalidArgumentException( const char *pMessage ) : JVMXException( pMessage )
  {}

  InvalidArgumentException( const char *pMessage, int errorCode ) : JVMXException( pMessage, errorCode )
  {}

  virtual ~InvalidArgumentException() JVMX_NOEXCEPT{};
};

#endif // __INVALIDARGUMENTEXCEPTION_H__
