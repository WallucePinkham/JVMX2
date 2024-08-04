
#ifndef __OUTOFMEMORYEXCEPTION_H__
#define __OUTOFMEMORYEXCEPTION_H__

#include "JVMXException.h"

class OutOfMemoryException : public JVMXException
{
public:
  OutOfMemoryException( const char *pMessage ) : JVMXException( pMessage )
  {}

  OutOfMemoryException( const char *pMessage, int errorCode ) : JVMXException( pMessage, errorCode )
  {}

  virtual ~OutOfMemoryException() JVMX_NOEXCEPT {};
};

#endif // __OUTOFMEMORYEXCEPTION_H__
