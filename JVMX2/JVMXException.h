
#ifndef __JVMXEXCEPTION_H__
#define __JVMXEXCEPTION_H__

#include <exception>

#include "GlobalConstants.h"

class JVMXException : public std::exception
{
public:
  JVMXException( const char *pMessage ) : std::exception( pMessage )
  {}

  JVMXException( const char *pMessage, int errorCode ) : std::exception( pMessage, errorCode )
  {}

  virtual ~JVMXException() {};
};


#endif // __JVMXEXCEPTION_H__
