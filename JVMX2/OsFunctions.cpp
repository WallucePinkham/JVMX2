
#include "OsFunctions.h"
//#include "OperatingSystemWindows.h"
#include "JavaNativeInterfaceLibrary.h"
#include "OsFunctionsSingletonFactory.h"

OsFunctions *OsFunctions::s_pInstance = nullptr;

intptr_t OsFunctions::GetProcessID()
{
  return m_pDelegate->GetProcessID();
}

std::string OsFunctions::GetHostName()
{
  return m_pDelegate->GetHostName();
}

void OsFunctions::SetThreadName( const char *name )
{
  return m_pDelegate->SetThreadName( name );
}

OsFunctions::~OsFunctions()
{
  delete s_pInstance;
}

std::wstring OsFunctions::ConvertCharToWChar( const char *input )
{
  return m_pDelegate->ConvertCharToWChar( input );
}

OsFunctions &OsFunctions::GetInstance()
{
  if ( nullptr == s_pInstance )
  {
    s_pInstance = new OsFunctions( OsFunctionsSingletonFactory::GetOsDelegate() );
  }

  return *s_pInstance;
}

void OsFunctions::ReplaceDelegate( IOperatingSystemDelegate *pDelegate )
{
  delete m_pDelegate;
  m_pDelegate = pDelegate;
}

OsFunctions::OsFunctions( IOperatingSystemDelegate *pDelegate )
{
  m_pDelegate = pDelegate;
}

std::string OsFunctions::ConvertCharToUTF8( const char *input )
{
  return m_pDelegate->ConvertCharToUTF8( input );
}

std::string OsFunctions::ConvertWCharToUTF8( const wchar_t *input )
{
  return m_pDelegate->ConvertWCharToUTF8( input );
}

std::wstring OsFunctions::ConvertUTF8ToWChar( const std::string &input )
{
  return m_pDelegate->ConvertCharToWChar( input );
}

bool OsFunctions::IsDirectory( const JVMX_ANSI_CHAR_TYPE *pFunctionName )
{
  return m_pDelegate->IsDirectory( pFunctionName );
}

void *OsFunctions::FindFunction( const JavaNativeInterfaceLibrary &lib, const JVMX_ANSI_CHAR_TYPE *pFunctionName )
{
  return m_pDelegate->FindFunction( lib, pFunctionName );
}

std::string OsFunctions::MapLibraryName( const std::string &shortName )
{
  return m_pDelegate->MapLibraryName( shortName );
}

std::shared_ptr<JavaNativeInterfaceLibrary> OsFunctions::LoadOsLibrary( const JVMX_ANSI_CHAR_TYPE *pLibraryName )
{
  return m_pDelegate->LoadOsLibrary( pLibraryName );
}

const char *OsFunctions::GetPathAnsi()
{
  return m_pDelegate->GetPathAnsi();
}

void OsFunctions::SuspendThread( std::thread::native_handle_type threadHandle )
{
  m_pDelegate->SuspendThread( threadHandle );
}

void OsFunctions::ResumeThread( std::thread::native_handle_type threadHandle )
{
  m_pDelegate->ResumeThread( threadHandle );
}

