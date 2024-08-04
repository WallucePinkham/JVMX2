#ifndef _FILELOGGER__H_
#define _FILELOGGER__H_

#include "ILogger.h"

class FileLogger : public ILogger
{
public:
  FileLogger( const JVMX_ANSI_CHAR_TYPE *pFileName );
  virtual ~FileLogger();

  virtual void LogError( const JVMX_ANSI_CHAR_TYPE *pMessage, ... ) JVMX_OVERRIDE;
  virtual void LogWarning( const JVMX_ANSI_CHAR_TYPE *pMessage, ... ) JVMX_OVERRIDE;
  virtual void LogInformation( const JVMX_ANSI_CHAR_TYPE *pMessage, ... ) JVMX_OVERRIDE;
  virtual void LogDebug( const JVMX_ANSI_CHAR_TYPE *pMessage, ... ) JVMX_OVERRIDE;

  void SetLogLevel( int level );

private:
  FILE *m_pFile;
  int m_LogLevel;
};

#endif // _FILELOGGER__H_
