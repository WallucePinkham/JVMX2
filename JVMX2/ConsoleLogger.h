#ifndef _CONSOLELOGGER__H_
#define _CONSOLELOGGER__H_

#include "ILogger.h"

class ConsoleLogger : public ILogger
{
public:
  ConsoleLogger();
  virtual ~ConsoleLogger() {};

  virtual void LogError( const JVMX_ANSI_CHAR_TYPE *pMessage, ... ) JVMX_OVERRIDE;
  virtual void LogWarning( const JVMX_ANSI_CHAR_TYPE *pMessage, ... ) JVMX_OVERRIDE;
  virtual void LogInformation( const JVMX_ANSI_CHAR_TYPE *pMessage, ... ) JVMX_OVERRIDE;
  virtual void LogDebug( const JVMX_ANSI_CHAR_TYPE *pMessage, ... ) JVMX_OVERRIDE;
};

#endif // _CONSOLELOGGER__H_
