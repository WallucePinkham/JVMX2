#include <cstdarg>

#include "InvalidArgumentException.h"

#include "AgregateLogger.h"

const size_t c_BufferSize = 1024 * 10;

void AgregateLogger::LogInformation( const JVMX_ANSI_CHAR_TYPE *pMessage, ... )
{
  va_list args;
  va_start( args, pMessage );

  char buffer[ c_BufferSize ] = { 0 };
  _vsnprintf( buffer, c_BufferSize - 1, pMessage, args );

  for (auto pLogger : m_Loggers)
  {
    pLogger->LogInformation( "%s", buffer );
  }

  va_end( args );
}

void AgregateLogger::LogWarning( const JVMX_ANSI_CHAR_TYPE *pMessage, ... )
{
  va_list args;
  va_start( args, pMessage );

  char buffer[ c_BufferSize ] = { 0 };
  _vsnprintf( buffer, c_BufferSize - 1, pMessage, args );

  for (auto pLogger : m_Loggers)
  {
    pLogger->LogWarning( "%s", buffer );
  }

  va_end( args );
}

void AgregateLogger::LogError( const JVMX_ANSI_CHAR_TYPE *pMessage, ... )
{
  va_list args;
  va_start( args, pMessage );

  char buffer[ c_BufferSize ];
  _vsnprintf( buffer, c_BufferSize - 1, pMessage, args );

  for (auto pLogger : m_Loggers)
  {
    pLogger->LogError( "%s", buffer );
  }

  va_end( args );
}

void AgregateLogger::LogDebug( const JVMX_ANSI_CHAR_TYPE *pMessage, ... )
{
  va_list args;
  va_start( args, pMessage );

  char buffer[ c_BufferSize ];
  _vsnprintf( buffer, c_BufferSize - 1, pMessage, args );

  for (auto pLogger : m_Loggers)
  {
    pLogger->LogDebug( "%s", buffer );
  }

  va_end( args );
}

void AgregateLogger::AddLogger( std::shared_ptr<ILogger> pLogger )
{
  if ( nullptr == pLogger )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Logger can't be a NULL pointer" );
  }

  m_Loggers.push_back( pLogger );
}