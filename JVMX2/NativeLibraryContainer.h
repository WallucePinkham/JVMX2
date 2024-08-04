
#pragma once

#ifndef _NATIVELIBRARYCONTAINER__H_
#define _NATIVELIBRARYCONTAINER__H_

#include <list>
#include <wallaroo/part.h>

//#include "JavaNativeInterfaceLibrary.h"

class JavaNativeInterfaceLibrary;

class NativeLibraryContainer : public wallaroo::Part
{
public:
  void Add( std::shared_ptr<JavaNativeInterfaceLibrary> );
  void *FindFunction( const char *pName );

private:
  std::list< std::shared_ptr<JavaNativeInterfaceLibrary> > m_Libraries;
};

 
#endif // _NATIVELIBRARYCONTAINER__H_


