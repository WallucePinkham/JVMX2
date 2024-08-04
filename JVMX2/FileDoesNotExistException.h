
#ifndef __FILEDOESNOTEXISTEXCEPTION_H__
#define __FILEDOESNOTEXISTEXCEPTION_H__

#include "JVMXException.h"

class FileDoesNotExistException : public JVMXException
{
public:
  FileDoesNotExistException( const char *pMessage ) : JVMXException( pMessage )
  {}

  FileDoesNotExistException( const char *pMessage, int errorCode ) : JVMXException( pMessage, errorCode )
  {}

  virtual ~FileDoesNotExistException() JVMX_NOEXCEPT{};
};

#endif // __FILEDOESNOTEXISTEXCEPTION_H__
