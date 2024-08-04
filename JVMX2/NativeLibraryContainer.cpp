#include "JavaNativeInterfaceLibrary.h"
#include "NativeLibraryContainer.h"
#include "OsFunctions.h"




void NativeLibraryContainer::Add( std::shared_ptr<JavaNativeInterfaceLibrary> pLibrary )
{
  m_Libraries.push_back( pLibrary );
}

void *NativeLibraryContainer::FindFunction( const char *pName )
{
  void *pFoundFunction = nullptr;
  for ( std::shared_ptr<JavaNativeInterfaceLibrary> pLib : m_Libraries )
  {
    pFoundFunction = OsFunctions::GetInstance().FindFunction( *pLib, reinterpret_cast<const JVMX_ANSI_CHAR_TYPE *>(pName) );
    if (nullptr!=pFoundFunction )
    {
      return pFoundFunction;
    }
  }

  return nullptr;
}
