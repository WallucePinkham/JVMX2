
#ifndef __INDEXOUTOFBOUNDSEXCEPTION_H__
#define __INDEXOUTOFBOUNDSEXCEPTION_H__

#include "JVMXException.h"

class IndexOutOfBoundsException : public JVMXException
{
public:
  IndexOutOfBoundsException( const char *pMessage ) : JVMXException( pMessage )
  {}

  IndexOutOfBoundsException( const char *pMessage, int errorCode ) : JVMXException( pMessage, errorCode )
  {}

  virtual ~IndexOutOfBoundsException() JVMX_NOEXCEPT{};
};

#endif // __INDEXOUTOFBOUNDSEXCEPTION_H__
