#include <windows.h>
#include <Shlwapi.h>

#include "JavaNativeInterfaceLibrary.h"

#include "CharacterCoversionFailedException.h"
#include "InternalErrorException.h"

#include "OperatingSystemWindows.h"


#pragma pack(push,8)  
typedef struct tagTHREADNAME_INFO
{
  DWORD dwType; // Must be 0x1000.  
  LPCSTR szName; // Pointer to name (in user addr space).  
  DWORD dwThreadID; // Thread ID (-1=caller thread).  
  DWORD dwFlags; // Reserved for future use, must be zero.  
} THREADNAME_INFO;
#pragma pack(pop)  

const int c_CouldNotDetermineBufferLength = 0xFFFD;
const DWORD MS_VC_EXCEPTION = 0x406D1388;

OperatingSystemWindows::OperatingSystemWindows()
{}

OperatingSystemWindows::~OperatingSystemWindows()
{}

std::wstring OperatingSystemWindows::ConvertCharToWChar( const std::string &input )
{
  std::wstring result;
  int bytesRequired = MultiByteToWideChar( CP_UTF8, 0, input.c_str(), -1, NULL, 0 );

  if ( c_CouldNotDetermineBufferLength == bytesRequired )
  {
    throw CharacterCoversionFailedException( __FUNCTION__ " - Could not get buffer length.", bytesRequired );
  }

  WCHAR *pBuffer = nullptr;

  try
  {
    pBuffer = new WCHAR[ bytesRequired ];

    int conversionResult = MultiByteToWideChar( CP_UTF8, 0, input.c_str(), -1, pBuffer, bytesRequired );
    if ( 0 == conversionResult )
    {
      throw CharacterCoversionFailedException( __FUNCTION__ " - Conversion failed.", GetLastError() );
    }
  }
  catch ( ... )
  {
    delete[] pBuffer;
    throw;
  }

  result.assign( pBuffer );
  delete[] pBuffer;

  return result;
}

std::string OperatingSystemWindows::ConvertCharToUTF8( const std::string &input )
{
  return input;
}

std::string OperatingSystemWindows::ConvertWCharToUTF8( const std::wstring &input )
{
  std::string mbResult;

  int bytesRequired = WideCharToMultiByte( CP_UTF8, WC_ERR_INVALID_CHARS, input.c_str(), -1, NULL, 0, NULL, NULL );

  if ( c_CouldNotDetermineBufferLength == bytesRequired )
  {
    throw CharacterCoversionFailedException( __FUNCTION__ " - Could not get buffer length.", bytesRequired );
  }

  CHAR *pBuffer = nullptr;

  try
  {
    pBuffer = new CHAR[ bytesRequired ];

    int conversionResult = WideCharToMultiByte( CP_UTF8, WC_ERR_INVALID_CHARS, input.c_str(), -1, pBuffer, bytesRequired, NULL, NULL );
    if ( 0 == conversionResult )
    {
      throw CharacterCoversionFailedException( __FUNCTION__ " - Conversion failed.", GetLastError() );
    }
  }
  catch ( ... )
  {
    delete[] pBuffer;
    throw;
  }

  mbResult.assign( pBuffer );
  delete[] pBuffer;

  return mbResult;
}

bool OperatingSystemWindows::IsDirectory( const JVMX_ANSI_CHAR_TYPE *pFunctionName )
{
  DWORD dwAttributes = GetFileAttributesA( pFunctionName );
  if ( INVALID_FILE_ATTRIBUTES != dwAttributes &&
    (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) )
  {
    return true;
  }

  return false;
} 

void *OperatingSystemWindows::FindFunction( const JavaNativeInterfaceLibrary &lib, const JVMX_ANSI_CHAR_TYPE *pFunctionName )
{
  void *pResult = GetProcAddress( lib.m_ModuleHandle, pFunctionName );

#ifdef _DEBUG
  if ( NULL == pResult )
  {
    DWORD err = GetLastError();
    return nullptr;
  }
#endif // _DEBUG


  return pResult;
}

std::shared_ptr<JavaNativeInterfaceLibrary> OperatingSystemWindows::LoadOsLibrary( const JVMX_ANSI_CHAR_TYPE *pLibraryName )
{
  HMODULE hResult = LoadLibraryA( pLibraryName );
#ifdef _DEBUG
  if ( NULL == hResult )
  {
    DWORD err = GetLastError();
    return nullptr;
  }
#endif // _DEBUG

//  JavaNativeInterfaceLibrary lib;
 
  
  auto pLib =   std::make_shared<JavaNativeInterfaceLibrary>();
  pLib->m_ModuleHandle = hResult;

  return pLib;
}

const char *OperatingSystemWindows::GetPathAnsi()
{
  return getenv( "PATH" );
}

void OperatingSystemWindows::SuspendThread( std::thread::native_handle_type threadHandle )
{
  ::SuspendThread( reinterpret_cast<HANDLE>(threadHandle) );
}

void OperatingSystemWindows::ResumeThread( std::thread::native_handle_type threadHandle )
{
  ::ResumeThread( reinterpret_cast<HANDLE>(threadHandle) );
}

std::string OperatingSystemWindows::MapLibraryName( const std::string &shortName )
{
  return shortName + ".dll";
}

void OperatingSystemWindows::SetThreadName( const char *pName )
{
  THREADNAME_INFO info;
  info.dwType = 0x1000;
  info.szName = pName;
  info.dwThreadID = GetCurrentThreadId();
  info.dwFlags = 0;
#pragma warning(push)  
#pragma warning(disable: 6320 6322)
  __try
  {
    RaiseException( MS_VC_EXCEPTION, 0, sizeof( info ) / sizeof( ULONG_PTR ), (ULONG_PTR*)&info );
  }
  __except ( EXCEPTION_EXECUTE_HANDLER )
  {}
#pragma warning(pop)    
}

intptr_t OperatingSystemWindows::GetProcessID()
{
  return static_cast<intptr_t>( ::GetCurrentProcessId() );
}

std::string OperatingSystemWindows::GetHostName()
{
  char pBuffer[ MAX_COMPUTERNAME_LENGTH + 1 ] = { 0 };
  DWORD length = MAX_COMPUTERNAME_LENGTH + 1;

  if ( !GetComputerNameA( pBuffer, &length ) )
  {
    throw InternalErrorException( __FUNCTION__ " - Could not get host name. " );
  }

  return std::string(pBuffer);
}

const char* OperatingSystemWindows::GetPathSeparator()
{
  return ";";
}

const char* OperatingSystemWindows::GetLineSeparator()
{
  return "\r\n";
}

const char* OperatingSystemWindows::GetFileSeparator()
{
  return "\\";
}

std::u16string OperatingSystemWindows::GetCanonicalFormUtf16(const JVMX_WIDE_CHAR_TYPE *pRelativePath)
{
  // On Windows, we can assume that wchar_t and char16_t are the same size.
  wchar_t absolutePath[MAX_PATH];

  // Java can send through the string with a leading slash because of how file:\\ type URLs are handled internally 
  if (pRelativePath[0] == u'\\')
  {
    ++pRelativePath;
  }

  DWORD result = GetFullPathName(reinterpret_cast<const wchar_t *>(pRelativePath), MAX_PATH, absolutePath, NULL);
  
  if (0 == result)
  {
    throw InternalErrorException(__FUNCTION__ " - Could not get absolute path from relative path. ");
  }

  return std::u16string(reinterpret_cast<const char16_t*>(absolutePath));
}