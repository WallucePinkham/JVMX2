
#ifndef _INTERNALERROREXCEPTION__H_
#define _INTERNALERROREXCEPTION__H_

#include "JVMXException.h"

class InternalErrorException : public JVMXException
{
public:
  InternalErrorException( const char *pMessage ) : JVMXException( pMessage )
  {}

  InternalErrorException( const char *pMessage, int errorCode ) : JVMXException( pMessage, errorCode )
  {}

  virtual ~InternalErrorException() JVMX_NOEXCEPT{};
};

#endif // _INTERNALERROREXCEPTION__H_
