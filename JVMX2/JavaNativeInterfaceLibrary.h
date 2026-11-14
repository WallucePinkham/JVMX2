#pragma once

#ifndef _JAVANATIVEINTERFACELIBRARY__H_
#define _JAVANATIVEINTERFACELIBRARY__H_

#if defined(_WIN32) || defined (_WIN64)
#include <windows.h>
#elif defined(__linux__)
#include <dlfcn.h>
#else
#error "You need to implement a JavaNativeInterfaceLibrary for your OS."
#endif

class JavaNativeInterfaceLibrary
{
public:
  JavaNativeInterfaceLibrary() : m_ModuleHandle( nullptr )
  {}

  virtual ~JavaNativeInterfaceLibrary()
  {
#if defined(_WIN32) || defined (_WIN64)
    if ( nullptr != m_ModuleHandle )
    {
      FreeLibrary( m_ModuleHandle );
    }
#elif defined(__linux__)
    if ( nullptr != m_ModuleHandle )
    {
      dlclose( m_ModuleHandle );
    }
#endif
  }

#if defined(_WIN32) || defined (_WIN64)
  HMODULE m_ModuleHandle;
#elif defined(__linux__)
  void *m_ModuleHandle;
#else
#error "You need to implement a JavaNativeInterfaceLibrary for your OS. Create a value called m_ModuleHandle."
#endif
};

#endif // _JAVANATIVEINTERFACELIBRARY__H_
