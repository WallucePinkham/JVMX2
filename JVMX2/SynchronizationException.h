
#ifndef __SYNCHRONIZATIONEXCEPTION_H__
#define __SYNCHRONIZATIONEXCEPTION_H__

#include "JVMXException.h"

class SynchronizationException : public JVMXException
{
public:
  SynchronizationException( const char *pMessage ) : JVMXException( pMessage )
  {}

  SynchronizationException( const char *pMessage, int errorCode ) : JVMXException( pMessage, errorCode )
  {}

  virtual ~SynchronizationException() JVMX_NOEXCEPT {};
};

#endif // __SYNCHRONIZATIONEXCEPTION_H__
