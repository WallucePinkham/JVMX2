
#ifndef _JAVANATIVEINTERFACELIBRARY__H_
#define _JAVANATIVEINTERFACELIBRARY__H_

#if defined(_WIN32) || defined (_WIN64)
#include <windows.h>
#else
#error "You need to implement a JavaNativeInterfaceLibrary for your OS."
#endif

class JavaNativeInterfaceLibrary
{
public:
  virtual ~JavaNativeInterfaceLibrary()
  {
#if defined(_WIN32) || defined (_WIN64)
    FreeLibrary( m_ModuleHandle );
#endif
  }

#if defined(_WIN32) || defined (_WIN64)
  HMODULE m_ModuleHandle;
#else
#error "You need to implement a JavaNativeInterfaceLibrary for your OS. Create a value called m_ModuleHandle."
#endif
};

#endif // _JAVANATIVEINTERFACELIBRARY__H_
