
#ifndef __TYPEMISMATCHEXCEPTION_H__
#define __TYPEMISMATCHEXCEPTION_H__

#include "JVMXException.h"

class TypeMismatchException : public JVMXException
{
public:
  TypeMismatchException( const char *pMessage ) : JVMXException( pMessage )
  {}

  TypeMismatchException( const char *pMessage, int errorCode ) : JVMXException( pMessage, errorCode )
  {}

  virtual ~TypeMismatchException() JVMX_NOEXCEPT{};
};
#endif // __TYPEMISMATCHEXCEPTION_H__
