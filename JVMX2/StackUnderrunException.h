
#ifndef _STACKUNDERFLOWEXCEPTION__H_
#define _STACKUNDERFLOWEXCEPTION__H_

#include "JVMXException.h"

class StackUnderrunException : public JVMXException
{
public:
  StackUnderrunException( const char *pMessage ) : JVMXException( pMessage )
  {}

  StackUnderrunException( const char *pMessage, int errorCode ) : JVMXException( pMessage, errorCode )
  {}

  virtual ~StackUnderrunException() JVMX_NOEXCEPT{};
};


#endif // _STACKUNDERFLOWEXCEPTION__H_
