
#include <cstdio>
#include <cstdarg>
#include <thread>

#include "FileDoesNotExistException.h"
#include "FileLogger.h"


FileLogger::FileLogger(const JVMX_ANSI_CHAR_TYPE *pFileName )
{
  m_LogLevel = 0;

  m_pFile = fopen( pFileName, "wb" );
  if ( nullptr == m_pFile )
  {
    throw FileDoesNotExistException( "Could not open log file." );
  }
}

void FileLogger::LogInformation( const JVMX_ANSI_CHAR_TYPE *pMessage, ... )
{
  if ( m_LogLevel < 3 )
  {
    return;
  }

  va_list args;
  va_start( args, pMessage );

  fprintf( m_pFile, "!I " );
  vfprintf( m_pFile, pMessage, args );
  fprintf( m_pFile, "\n" );

  fflush( m_pFile );

  va_end( args );
}

void FileLogger::LogWarning( const JVMX_ANSI_CHAR_TYPE *pMessage, ... )
{
  if ( m_LogLevel < 2 )
  {
    return;
  }

  va_list args;
  va_start( args, pMessage );

  fprintf( m_pFile, "!W " );
  vfprintf( m_pFile, pMessage, args );
  fprintf( m_pFile, "\n" );

  fflush( m_pFile );

  va_end( args );
}

void FileLogger::LogError( const JVMX_ANSI_CHAR_TYPE *pMessage, ... )
{
  va_list args;
  va_start( args, pMessage );

  fprintf( m_pFile, "!E " );
  vfprintf( m_pFile, pMessage, args );
  fprintf( m_pFile, "\n" );

  fflush( m_pFile );

  va_end( args );
}

void FileLogger::LogDebug( const JVMX_ANSI_CHAR_TYPE *pMessage, ... )
{
  if ( m_LogLevel < 4 )
  {
    return;
  }

  va_list args;
  va_start( args, pMessage );

  fprintf( m_pFile, "!D [%lu] ", std::hash<std::thread::id> {} (std::this_thread::get_id()) );
  vfprintf( m_pFile, pMessage, args );
  fprintf( m_pFile, "\n" );

  fflush( m_pFile );

  va_end( args );
}

void FileLogger::SetLogLevel( int level )
{
  m_LogLevel = level;
}

FileLogger::~FileLogger()
{
  fclose( m_pFile );
}
