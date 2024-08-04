
#ifndef __UNSUPPORTEDOPERATIONEXCEPTION_H__
#define __UNSUPPORTEDOPERATIONEXCEPTION_H__

#include "JVMXException.h"

class UnsupportedOperationException : public JVMXException
{
public:
  UnsupportedOperationException( const char *pMessage ) : JVMXException( pMessage )
  {}

  UnsupportedOperationException( const char *pMessage, int errorCode ) : JVMXException( pMessage, errorCode )
  {}

  virtual ~UnsupportedOperationException() JVMX_NOEXCEPT{};
};

#endif // __UNSUPPORTEDOPERATIONEXCEPTION_H__
