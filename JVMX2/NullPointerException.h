
#ifndef __NULLPOINTEREXCEPTION_H__
#define __NULLPOINTEREXCEPTION_H__

#include "JVMXException.h"

class NullPointerException : public JVMXException
{
public:
  NullPointerException( const char *pMessage ) : JVMXException( pMessage )
  {}

  NullPointerException( const char *pMessage, int errorCode ) : JVMXException( pMessage, errorCode )
  {}

  virtual ~NullPointerException() JVMX_NOEXCEPT{};
};

#endif // __NULLPOINTEREXCEPTION_H__
