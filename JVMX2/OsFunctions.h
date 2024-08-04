
#ifndef __OSFUNCTIONS_H__
#define __OSFUNCTIONS_H__

#include <thread>
#include <memory>

#include "OsFunctionsSingletonFactory.h"
#include "IOperatingSystemDelegate.h"

class OsFunctions
{
  //friend class OsFunctionsSingletonFactory;

private:
  // This assumes that the class will take over the pointer and call 'delete' on it when it's done.
  OsFunctions( IOperatingSystemDelegate *pDelegate );

public:
  virtual ~OsFunctions();

  // This assumes that the class will take over the pointer and call 'delete' on it when it's done.
  void ReplaceDelegate( IOperatingSystemDelegate *pDelegate );

  std::wstring ConvertCharToWChar( const char *input );
  std::string ConvertCharToUTF8( const char *input );
  std::string ConvertWCharToUTF8( const wchar_t *input );
  std::wstring ConvertUTF8ToWChar( const std::string &input );

  bool IsDirectory( const JVMX_ANSI_CHAR_TYPE *pFunctionName );

  void *FindFunction( const JavaNativeInterfaceLibrary &lib, const JVMX_ANSI_CHAR_TYPE *pFunctionName );

  std::string MapLibraryName( const std::string &shortName );
  std::shared_ptr<JavaNativeInterfaceLibrary> LoadOsLibrary( const JVMX_ANSI_CHAR_TYPE * pLibraryName );

  static OsFunctions &GetInstance();
  const char * GetPathAnsi();

  void SuspendThread( std::thread::native_handle_type threadHandle );
  void ResumeThread( std::thread::native_handle_type threadHandle );

  void SetThreadName( const char *name );

  intptr_t GetProcessID();
  std::string GetHostName();

private:
  IOperatingSystemDelegate *m_pDelegate;

  static OsFunctions *s_pInstance;
};

#endif // __OSFUNCTIONS_H__
