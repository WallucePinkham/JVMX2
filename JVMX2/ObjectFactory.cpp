// #include "InvalidArgumentException.h"
// 
// #include "ObjectFactory.h"
// #include "JavaObject.h"
// #include "JavaArray.h"
// 
// boost::intrusive_ptr<JavaObject> ObjectFactory::AllocateObject( std::shared_ptr<IMemoryManager> pMemoryManager, std::shared_ptr<JavaClass> pClass )
// {
//   if ( nullptr == pMemoryManager )
//   {
//     throw InvalidArgumentException( __FUNCTION__ " - Memory Manager Pointer cannot be NULL." );
//   }
// 
//   if ( nullptr == JavaObject::FieldAllocatorType::GetAllocator() )
//   {
//     JavaObject::FieldAllocatorType::SetAllocator( pMemoryManager );
//   }
// 
//   if ( nullptr == JavaObject::JVMXFieldAllocatorType::GetAllocator() )
//   {
//     JavaObject::JVMXFieldAllocatorType::SetAllocator( pMemoryManager );
//   }
// 
//   uint8_t *pMemoryBlock = pMemoryManager->Allocate( sizeof( JavaObject ) + sizeof( intptr_t ) );
//   boost::intrusive_ptr<JavaObject> pObject = nullptr;
//   try
//   {
//     // Placement New
//     IMemoryManager *pNativeMemoryManagerPointer = pMemoryManager.get();
//     memcpy( pMemoryBlock, &pNativeMemoryManagerPointer, sizeof( intptr_t ) );
// 
//     void *pPointer = pMemoryBlock + sizeof( intptr_t );
//     pObject = new (pPointer) JavaObject( pClass );
//   }
//   catch ( ... )
//   {
//     pMemoryManager->Free( pMemoryBlock );
//     throw;
//   }
// 
//   return pObject;
// }
// 
// void ObjectFactory::FreeObject( void *pObject )
// {
//   IMemoryManager *pMemoryManager = GetObjectMemoryManager( pObject );
//   pMemoryManager->Free( GetMemoryBlockStart( pObject ) );
// }
// 
// boost::intrusive_ptr<ObjectReference> ObjectFactory::AllocateArray( std::shared_ptr<IMemoryManager> pMemoryManager, e_JavaArrayTypes type, size_t size )
// {
//   if ( nullptr == pMemoryManager )
//   {
//     throw InvalidArgumentException( __FUNCTION__ " - Memory Manager Pointer cannot be NULL." );
//   }
// 
//   if ( nullptr == JavaArray::ElementAllocator::GetAllocator() )
//   {
//     JavaArray::ElementAllocator::SetAllocator( pMemoryManager );
//   }
// 
//   uint8_t *pMemoryBlock = pMemoryManager->Allocate( sizeof( JavaArray ) + sizeof( intptr_t ) );
//   boost::intrusive_ptr<ObjectReference> pObject = nullptr;
//   try
//   {
//     // Placement New
//     IMemoryManager *pNativeMemoryManagerPointer = pMemoryManager.get();
//     memcpy( pMemoryBlock, &pNativeMemoryManagerPointer, sizeof( intptr_t ) );
// 
//     void *pPointer = pMemoryBlock + sizeof( intptr_t );
//     pObject = new (pPointer) JavaArray( pMemoryManager, type, size );
//   }
//   catch ( ... )
//   {
//     pMemoryManager->Free( pMemoryBlock );
//     throw;
//   }
// 
//   return pObject;
// }
// 
// void ObjectFactory::FreeArray( void *pArray )
// {
//   FreeObject( pArray );
// }
// 
// IMemoryManager *ObjectFactory::GetObjectMemoryManager( boost::intrusive_ptr<JavaObject> pObject )
// {
//   return GetObjectMemoryManager( reinterpret_cast<void *>(pObject.get()) );
// }
// 
// IMemoryManager *ObjectFactory::GetObjectMemoryManager( void *pObject )
// {
//   uint8_t *pMemoryBlock = GetMemoryBlockStart( pObject );
//   IMemoryManager *pMemoryManager = nullptr;
//   memcpy( &pMemoryManager, pMemoryBlock, sizeof( intptr_t ) );
//   return pMemoryManager;
// }
// 
// uint8_t * ObjectFactory::GetMemoryBlockStart( void * pObject )
// {
//   uint8_t *pBytePointer = static_cast<uint8_t *>(pObject);
//   return  pBytePointer - sizeof( intptr_t );
// }
// 
