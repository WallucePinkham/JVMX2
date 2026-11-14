#include <arpa/inet.h>
#include <codecvt>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <limits.h>
#include <locale>
#include <memory>
#include <string>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#include "JavaNativeInterfaceLibrary.h"
#include "InternalErrorException.h"
#include "NotImplementedException.h"
#include "OperatingSystemLinux.h"

namespace
{
  size_t GetUtf8PrefixLength( const std::string &text, size_t maxLength )
  {
    size_t length = text.size() < maxLength ? text.size() : maxLength;
    if ( length == text.size() )
    {
      return length;
    }

    size_t sequenceStart = length;
    while ( sequenceStart > 0 &&
      ( static_cast<unsigned char>( text[sequenceStart] ) & 0xC0 ) == 0x80 )
    {
      --sequenceStart;
    }

    unsigned char leadingByte = static_cast<unsigned char>( text[sequenceStart] );
    size_t sequenceLength = 1;
    if ( ( leadingByte & 0xE0 ) == 0xC0 )
    {
      sequenceLength = 2;
    }
    else if ( ( leadingByte & 0xF0 ) == 0xE0 )
    {
      sequenceLength = 3;
    }
    else if ( ( leadingByte & 0xF8 ) == 0xF0 )
    {
      sequenceLength = 4;
    }

    if ( sequenceStart + sequenceLength > length )
    {
      return sequenceStart;
    }

    return length;
  }

  std::string NormalizeAbsolutePath( const std::string &path )
  {
    std::vector<std::string> components;
    size_t componentStart = 0;

    while ( componentStart <= path.size() )
    {
      size_t separator = path.find( '/', componentStart );
      size_t componentEnd = separator == std::string::npos ? path.size() : separator;
      std::string component = path.substr( componentStart, componentEnd - componentStart );

      if ( component == ".." )
      {
        if ( !components.empty() )
        {
          components.pop_back();
        }
      }
      else if ( !component.empty() && component != "." )
      {
        components.push_back( component );
      }

      if ( separator == std::string::npos )
      {
        break;
      }
      componentStart = separator + 1;
    }

    std::string result = "/";
    for ( size_t index = 0; index < components.size(); ++index )
    {
      if ( index != 0 )
      {
        result += "/";
      }
      result += components[index];
    }

    return result;
  }
}

std::wstring OperatingSystemLinux::ConvertCharToWChar( const std::string &input )
{
  std::wstring_convert<std::codecvt_utf8<wchar_t> > converter;
  return converter.from_bytes( input );
}

std::string OperatingSystemLinux::ConvertCharToUTF8( const std::string &input )
{
  return input;
}

std::string OperatingSystemLinux::ConvertWCharToUTF8( const std::wstring &input )
{
  std::wstring_convert<std::codecvt_utf8<wchar_t> > converter;
  return converter.to_bytes( input );
}

bool OperatingSystemLinux::IsDirectory( const JVMX_ANSI_CHAR_TYPE *pFunctionName )
{
  struct stat fileStatus;
  return 0 == stat( pFunctionName, &fileStatus ) && S_ISDIR( fileStatus.st_mode );
}

void *OperatingSystemLinux::FindFunction( const JavaNativeInterfaceLibrary &lib, const JVMX_ANSI_CHAR_TYPE *pFunctionName )
{
  dlerror();
  void *pResult = dlsym( lib.m_ModuleHandle, pFunctionName );
  return nullptr == dlerror() ? pResult : nullptr;
}

std::shared_ptr<JavaNativeInterfaceLibrary> OperatingSystemLinux::LoadOsLibrary( const JVMX_ANSI_CHAR_TYPE *pLibraryName )
{
  void *pModule = dlopen( pLibraryName, RTLD_LAZY | RTLD_LOCAL );
  if ( nullptr == pModule )
  {
    return std::shared_ptr<JavaNativeInterfaceLibrary>();
  }

  std::shared_ptr<JavaNativeInterfaceLibrary> pLibrary( new JavaNativeInterfaceLibrary() );
  pLibrary->m_ModuleHandle = pModule;
  return pLibrary;
}

const char *OperatingSystemLinux::GetPathAnsi()
{
  return getenv( "PATH" );
}

void OperatingSystemLinux::SuspendThread( std::thread::native_handle_type )
{
  throw NotImplementedException( "OperatingSystemLinux::SuspendThread - Per-thread suspension is not supported." );
}

void OperatingSystemLinux::ResumeThread( std::thread::native_handle_type )
{
  throw NotImplementedException( "OperatingSystemLinux::ResumeThread - Per-thread resumption is not supported." );
}

std::string OperatingSystemLinux::MapLibraryName( const std::string &shortName )
{
  if ( shortName.size() >= 6 &&
    shortName.compare( 0, 3, "lib" ) == 0 &&
    shortName.compare( shortName.size() - 3, 3, ".so" ) == 0 )
  {
    return shortName;
  }

  return "lib" + shortName + ".so";
}

void OperatingSystemLinux::SetThreadName( const char *pName )
{
  std::string threadName( pName );
  threadName.resize( GetUtf8PrefixLength( threadName, 15 ) );

  if ( 0 != prctl( PR_SET_NAME, threadName.c_str(), 0, 0, 0 ) )
  {
    throw InternalErrorException( "OperatingSystemLinux::SetThreadName - Could not set thread name.", errno );
  }
}

intptr_t OperatingSystemLinux::GetProcessID()
{
  return static_cast<intptr_t>( getpid() );
}

std::string OperatingSystemLinux::GetHostName()
{
  char hostName[HOST_NAME_MAX + 1] = { 0 };
  if ( 0 != gethostname( hostName, sizeof( hostName ) ) )
  {
    throw InternalErrorException( "OperatingSystemLinux::GetHostName - Could not get host name.", errno );
  }

  hostName[HOST_NAME_MAX] = '\0';
  return std::string( hostName );
}

const char *OperatingSystemLinux::GetPathSeparator()
{
  return ":";
}

const char *OperatingSystemLinux::GetFileSeparator()
{
  return "/";
}

const char *OperatingSystemLinux::GetLineSeparator()
{
  return "\n";
}

std::u16string OperatingSystemLinux::GetCanonicalFormUtf16( const JVMX_WIDE_CHAR_TYPE *pRelativePath )
{
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
  std::string path = converter.to_bytes( pRelativePath );
  char *pResolvedPath = realpath( path.c_str(), nullptr );
  if ( nullptr != pResolvedPath )
  {
    std::unique_ptr<char, decltype( &free )> resolvedPath( pResolvedPath, &free );
    return converter.from_bytes( resolvedPath.get() );
  }

  std::string absolutePath = path;
  if ( path.empty() || path[0] != '/' )
  {
    char currentDirectory[PATH_MAX];
    if ( nullptr == getcwd( currentDirectory, sizeof( currentDirectory ) ) )
    {
      throw InternalErrorException( "OperatingSystemLinux::GetCanonicalFormUtf16 - Could not get current directory.", errno );
    }
    absolutePath = std::string( currentDirectory ) + "/" + path;
  }

  return converter.from_bytes( NormalizeAbsolutePath( absolutePath ) );
}

void OperatingSystemLinux::GetInetAddrAnyIpV4( uint8_t pBytes[4] )
{
  struct in_addr address;
  address.s_addr = htonl( INADDR_ANY );
  memcpy( pBytes, &address.s_addr, sizeof( address.s_addr ) );
}

bool OperatingSystemLinux::Ip4StringToBytes( const char *pIPAddress, uint8_t pBytes[4] )
{
  struct in_addr address;
  if ( 1 != inet_pton( AF_INET, pIPAddress, &address ) )
  {
    return false;
  }

  memcpy( pBytes, &address.s_addr, sizeof( address.s_addr ) );
  return true;
}
