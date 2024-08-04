
#ifndef _STACKOVERFLOWEXCEPTION__H_
#define _STACKOVERFLOWEXCEPTION__H_

#include "JVMXException.h"

class StackOverflowException : public JVMXException
{
public:
  StackOverflowException( const char *pMessage ) : JVMXException( pMessage )
  {}

  StackOverflowException( const char *pMessage, int errorCode ) : JVMXException( pMessage, errorCode )
  {}

  virtual ~StackOverflowException() JVMX_NOEXCEPT{};
};


#endif // _STACKOVERFLOWEXCEPTION__H_




