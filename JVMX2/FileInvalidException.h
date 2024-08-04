
#ifndef __FILEINVALIDEXCEPTION_H__
#define __FILEINVALIDEXCEPTION_H__

#include "JVMXException.h"

class FileInvalidException : public JVMXException
{
public:
  FileInvalidException( const char *pMessage ) : JVMXException( pMessage )
  {}

  FileInvalidException( const char *pMessage, int errorCode ) : JVMXException( pMessage, errorCode )
  {}

  virtual ~FileInvalidException() JVMX_NOEXCEPT{};
};

#endif // __FILEINVALIDEXCEPTION_H__
