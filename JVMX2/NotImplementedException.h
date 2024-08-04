
#ifndef _NOTIMPLEMENTEDEXCEPTION__H_
#define _NOTIMPLEMENTEDEXCEPTION__H_

#include "JVMXException.h"

class NotImplementedException : public JVMXException
{
public:
  NotImplementedException( const char *pMessage ) : JVMXException( pMessage )
  {}

  NotImplementedException( const char *pMessage, int errorCode ) : JVMXException( pMessage, errorCode )
  {}

  virtual ~NotImplementedException() JVMX_NOEXCEPT{};
};

#endif // _NOTIMPLEMENTEDEXCEPTION__H_
