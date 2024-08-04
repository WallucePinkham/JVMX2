#include <stdarg.h>
#include <cstdio>

#include "ConsoleLogger.h"

ConsoleLogger::ConsoleLogger()
{}

void ConsoleLogger::LogInformation( const JVMX_ANSI_CHAR_TYPE *pMessage, ... )
{
  va_list args;
  va_start( args, pMessage );

  fprintf( stdout, "!I " );
  vfprintf( stdout, pMessage, args );
  fprintf( stdout, "\n" );

  va_end( args );
}

void ConsoleLogger::LogWarning( const JVMX_ANSI_CHAR_TYPE *pMessage, ... )
{
  va_list args;
  va_start( args, pMessage );

  fprintf( stderr, "!W " );
  vfprintf( stderr, pMessage, args );
  fprintf( stderr, "\n" );

  va_end( args );
}

void ConsoleLogger::LogError( const JVMX_ANSI_CHAR_TYPE *pMessage, ... )
{
  va_list args;
  va_start( args, pMessage );

  fprintf( stderr, "!E " );
  vfprintf( stderr, pMessage, args );
  fprintf( stderr, "\n" );

  va_end( args );
}

void ConsoleLogger::LogDebug( const JVMX_ANSI_CHAR_TYPE *pMessage, ... )
{
  //   va_list args;
  //   va_start( args, pMessage );
  //
  //   fprintf( stdout, "!D " );
  //   vfprintf( stdout, pMessage, args );
  //   fprintf( stdout, "\n" );
  //
  //   va_end( args );
}