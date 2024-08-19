
#ifndef __OPERATINGSYSTEMWINDOWS_H__
#define __OPERATINGSYSTEMWINDOWS_H__

#include <memory>

#include "IOperatingSystemDelegate.h"

class OperatingSystemWindows : public IOperatingSystemDelegate
{
public:
  OperatingSystemWindows();
  virtual ~OperatingSystemWindows() JVMX_NOEXCEPT JVMX_OVERRIDE;

public:
  virtual std::wstring ConvertCharToWChar( const std::string &input ) JVMX_OVERRIDE;
  virtual std::string ConvertCharToUTF8( const std::string &input ) JVMX_OVERRIDE;
  virtual std::string ConvertWCharToUTF8( const std::wstring &input ) JVMX_OVERRIDE;

  virtual bool IsDirectory( const JVMX_ANSI_CHAR_TYPE *pFunctionName ) JVMX_OVERRIDE;

  virtual void *FindFunction( const JavaNativeInterfaceLibrary & lib, const JVMX_ANSI_CHAR_TYPE *pFunctionName ) JVMX_OVERRIDE;
  virtual std::shared_ptr<JavaNativeInterfaceLibrary> LoadOsLibrary( const JVMX_ANSI_CHAR_TYPE *pLibraryName ) JVMX_OVERRIDE;

  virtual const char *GetPathAnsi() JVMX_OVERRIDE;

  virtual void SuspendThread( std::thread::native_handle_type threadHandle ) JVMX_OVERRIDE;
  virtual void ResumeThread( std::thread::native_handle_type threadHandle ) JVMX_OVERRIDE;

  virtual std::string MapLibraryName( const std::string &shortName ) JVMX_OVERRIDE;

  virtual void SetThreadName( const char *pName ) JVMX_OVERRIDE;
  virtual intptr_t GetProcessID() JVMX_OVERRIDE;

  virtual std::string GetHostName() JVMX_OVERRIDE;

  virtual const char* GetPathSeparator() JVMX_OVERRIDE;
  virtual const char* GetFileSeparator() JVMX_OVERRIDE;
  virtual const char* GetLineSeparator() JVMX_OVERRIDE;

  std::u16string GetCanonicalFormUtf16(const JVMX_WIDE_CHAR_TYPE* pRelativePath) JVMX_OVERRIDE;
};

#endif // __OPERATINGSYSTEMWINDOWS_H__

