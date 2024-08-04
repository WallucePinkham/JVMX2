
#ifndef __IOPERATINGSYSTEMDELEGATE_H__
#define __IOPERATINGSYSTEMDELEGATE_H__

#include <string>
#include <thread>

#include "GlobalConstants.h"


class JavaNativeInterfaceLibrary;

class IOperatingSystemDelegate
{
public:
  virtual ~IOperatingSystemDelegate() JVMX_NOEXCEPT {};

  virtual std::wstring ConvertCharToWChar( const std::string &input ) JVMX_PURE;
  virtual std::string ConvertCharToUTF8( const std::string &input ) JVMX_PURE;
  virtual std::string ConvertWCharToUTF8( const std::wstring &input ) JVMX_PURE;

  virtual bool IsDirectory( const JVMX_ANSI_CHAR_TYPE *pFunctionName ) JVMX_PURE;

  virtual void *FindFunction( const JavaNativeInterfaceLibrary &lib, const JVMX_ANSI_CHAR_TYPE *pFunctionName ) JVMX_PURE;
  virtual std::shared_ptr<JavaNativeInterfaceLibrary> LoadOsLibrary( const JVMX_ANSI_CHAR_TYPE * pLibraryName ) JVMX_PURE;

  virtual const char *GetPathAnsi() JVMX_PURE;
  virtual void SuspendThread( std::thread::native_handle_type threadHandle ) JVMX_PURE;
  virtual void ResumeThread( std::thread::native_handle_type threadHandle ) JVMX_PURE;

  virtual std::string MapLibraryName( const std::string &shortName ) JVMX_PURE;

  virtual void SetThreadName( const char *pName ) JVMX_PURE;
  virtual intptr_t GetProcessID() JVMX_PURE;

  virtual std::string GetHostName() JVMX_PURE;
};

#endif // __IOPERATINGSYSTEMDELEGATE_H__
