#ifndef _AGREGATELOGGER__H_
#define _AGREGATELOGGER__H_

#include <memory>
#include <list>

#include "ILogger.h"

class AgregateLogger : public ILogger
{
public:
  AgregateLogger() {};
  virtual ~AgregateLogger() {};

  virtual void LogError( const JVMX_ANSI_CHAR_TYPE *pMessage, ... ) JVMX_OVERRIDE;
  virtual void LogWarning( const JVMX_ANSI_CHAR_TYPE *pMessage, ... ) JVMX_OVERRIDE;
  virtual void LogInformation( const JVMX_ANSI_CHAR_TYPE *pMessage, ... ) JVMX_OVERRIDE;
  virtual void LogDebug( const JVMX_ANSI_CHAR_TYPE *pMessage, ... ) JVMX_OVERRIDE;

  void AddLogger( std::shared_ptr<ILogger> pLogger );

private:
  std::list<std::shared_ptr<ILogger>> m_Loggers;
};

#endif // _AGREGATELOGGER__H_
