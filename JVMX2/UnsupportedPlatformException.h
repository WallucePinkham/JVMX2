
#ifndef __UNSUPPORTEDPLATFORMEXCEPTION_H__
#define __UNSUPPORTEDPLATFORMEXCEPTION_H__

#include "JVMXException.h"

class UnsupportedPlatformException : public JVMXException
{
public:
  UnsupportedPlatformException( const char *pMessage ) : JVMXException( pMessage )
  {}

  UnsupportedPlatformException( const char *pMessage, int errorCode ) : JVMXException( pMessage, errorCode )
  {}

  virtual ~UnsupportedPlatformException() JVMX_NOEXCEPT{};
};

#endif // __UNSUPPORTEDPLATFORMEXCEPTION_H__

