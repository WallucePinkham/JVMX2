
#ifndef _ILOGGER__H_
#define _ILOGGER__H_

#include "GlobalConstants.h"

#include "../wallaroo/wallaroo/part.h"

class ILogger JVMX_ABSTRACT : public wallaroo::Part
{
public:
  virtual ~ILogger() {}

  virtual void LogError( const JVMX_ANSI_CHAR_TYPE *pMessage, ... ) JVMX_PURE;
  virtual void LogWarning( const JVMX_ANSI_CHAR_TYPE *pMessage, ... ) JVMX_PURE;
  virtual void LogInformation( const JVMX_ANSI_CHAR_TYPE *pMessage, ... ) JVMX_PURE;
  virtual void LogDebug( const JVMX_ANSI_CHAR_TYPE *pMessage, ... ) JVMX_PURE;

protected:
  ILogger() {};
  ILogger( const ILogger & ) JVMX_FN_DELETE;
  ILogger &operator=(const ILogger &) JVMX_FN_DELETE;
};


#endif // _ILOGGER__H_
