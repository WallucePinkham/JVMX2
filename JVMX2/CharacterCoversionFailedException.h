
#ifndef __CHARACTERCOVERSIONFAILEDEXCEPTION_H__
#define __CHARACTERCOVERSIONFAILEDEXCEPTION_H__

#include "JVMXException.h"

class CharacterCoversionFailedException : public JVMXException
{
public:
  CharacterCoversionFailedException( const char *pMessage ) : JVMXException( pMessage )
  {}

  CharacterCoversionFailedException( const char *pMessage, int errorCode ) : JVMXException( pMessage, errorCode )
  {}

  virtual ~CharacterCoversionFailedException() JVMX_NOEXCEPT{};
};


#endif // __CHARACTERCOVERSIONFAILEDEXCEPTION_H__

