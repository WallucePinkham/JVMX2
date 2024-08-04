
#ifndef __UNSUPPORTEDJAVATYPEEXCEPTION_H__
#define __UNSUPPORTEDJAVATYPEEXCEPTION_H__

#include "JVMXException.h"

class UnsupportedTypeException : public JVMXException
{
public:
  UnsupportedTypeException( const char *pMessage ) : JVMXException( pMessage )
  {}

  UnsupportedTypeException( const char *pMessage, int errorCode ) : JVMXException( pMessage, errorCode )
  {}

  virtual ~UnsupportedTypeException() JVMX_NOEXCEPT{};
};

#endif // __UNSUPPORTEDJAVATYPEEXCEPTION_H__
