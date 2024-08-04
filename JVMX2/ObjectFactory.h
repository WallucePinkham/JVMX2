#ifndef _OBJECTFACTORY__H_
#define _OBJECTFACTORY__H_

#include "GlobalConstants.h"

#include <memory>
#include <boost/intrusive_ptr.hpp>

#include "JavaArrayTypes.h"

#include "IMemoryManager.h"

class JavaArray;
class JavaObject;
class JavaClass;

// class ObjectFactory
// {
// public:
//   static boost::intrusive_ptr<JavaObject> AllocateObject( std::shared_ptr<IMemoryManager> pMemoryManager, std::shared_ptr<JavaClass> pClass );
//   static void FreeObject( void *pObject ) throw();
// 
//   static boost::intrusive_ptr<ObjectReference> AllocateArray( std::shared_ptr<IMemoryManager> pMemoryManager, e_JavaArrayTypes type, size_t size );
//   static void FreeArray( void *pArray ) throw();
// 
//   static IMemoryManager *GetObjectMemoryManager( boost::intrusive_ptr<JavaObject> pObject );
// 
// private:
//   static IMemoryManager *GetObjectMemoryManager( void *pObject );
//   static uint8_t *GetMemoryBlockStart( void * pObject );
// 
// private:
//   ObjectFactory() JVMX_FN_DELETE;
//   ObjectFactory( const ObjectFactory &other ) JVMX_FN_DELETE;
// };

#endif // _OBJECTFACTORY__H_
