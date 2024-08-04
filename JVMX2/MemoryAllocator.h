// #ifndef _MEMORYALLOCATOR__H_
// #define _MEMORYALLOCATOR__H_
// 
// //#include <boost/intrusive_ptr.hpp>
// 
// #include "IGarbageCollector.h"
// 
// struct MemoryAllocatorDestruction
// {
//   static inline void destruct( char* ) {}
//   static inline void destruct( wchar_t* ) {}
// 
// #pragma warning(suppress:4100)
//   template< typename T >
//   static inline void destruct( T* t ) { t->~T(); }
// };
// 
// template <class T> class MemoryAllocator;
// 
// template <> class MemoryAllocator<void>
// {
// public:
//   typedef void*       pointer;
//   typedef const void* const_pointer;
//   // reference to void members are impossible.
//   typedef void value_type;
//   template <class U> struct rebind
//   {
//     typedef MemoryAllocator<U> other;
//   };
// };
// 
// template <class T>
// class MemoryAllocator
// {
// public:
//   typedef size_t    size_type;
//   typedef ptrdiff_t difference_type;
//   typedef T*        pointer;
//   typedef const T*  const_pointer;
//   typedef T&        reference;
//   typedef const T&  const_reference;
//   typedef T         value_type;
// 
//   template <class U> struct rebind
//   {
//     typedef MemoryAllocator<U> other;
//   };
// 
//   MemoryAllocator() throw()
//   {};
// 
// #pragma warning(suppress:4100)
//   MemoryAllocator( const MemoryAllocator &other ) throw()
//   {
//     m_pMemoryManager = other.m_pMemoryManager;
//   }
// 
// #pragma warning(suppress:4100)
//   template <class U> MemoryAllocator( const MemoryAllocator<U> &other ) throw()
//   {
//     this->SetAllocator( other.GetAllocator() );
//   }
// 
//   ~MemoryAllocator() throw() {};
// 
//   static void SetAllocator( std::shared_ptr<IGarbageCollector> pMemoryManager )
//   {
//     m_pMemoryManager = pMemoryManager;
//   }
// 
//   static std::shared_ptr<IGarbageCollector> GetAllocator()
//   {
//     return m_pMemoryManager;
//   }
// 
//   pointer address( reference x ) const { return &x; }
//   const_pointer address( const_reference x ) const { return &x; }
// 
//   pointer allocate( size_type size )
//   {
//     return reinterpret_cast<pointer>(m_pMemoryManager->AllocateBytes( size * sizeof( T ) ));
//   };
// 
//   pointer allocate( size_type size, MemoryAllocator<void>::const_pointer )
//   {
//     return reinterpret_cast<pointer>(m_pMemoryManager->AllocateBytes( size * sizeof( T ) ));
//   };
// 
//   void deallocate( pointer p, size_type )
//   {
//     //m_pMemoryManager->Free( reinterpret_cast<uint8_t *>(p) );
//   }
// 
//   static size_type max_size() throw() { return m_pMemoryManager->GetHeapSize(); }
// 
//   void construct( pointer p, const T& val )
//   {
//     new (p) T( val );
//   }
// 
//   void destroy( pointer p )
//   {
//     MemoryAllocatorDestruction::destruct( p );
//   }
// 
// protected:
//   static std::shared_ptr<IGarbageCollector> m_pMemoryManager;
// };
// 
// template <class T1, class T2>
// bool operator==( const MemoryAllocator<T1>&, const MemoryAllocator<T2>& ) throw() { return true; };
// 
// template <class T1, class T2>
// bool operator!=( const MemoryAllocator<T1>&, const MemoryAllocator<T2>& ) throw() { return false; };
// 
// template<class T>
// std::shared_ptr<IGarbageCollector> MemoryAllocator<T>::m_pMemoryManager = nullptr;
// 
// #endif // _MEMORYALLOCATOR__H_
