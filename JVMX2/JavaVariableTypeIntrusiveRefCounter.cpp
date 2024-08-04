
/*
#include "IJavaVariableType.h"
#include "JavaVariableTypeIntrusiveRefCounter.h"
#include "ObjectReference.h"

void intrusive_ptr_add_ref( const JavaVariableTypeIntrusiveRefCounter* p ) JVMX_NOEXCEPT
{
//   if ( e_JavaVariableTypes::Object != reinterpret_cast<const IJavaVariableType *>(p)->GetVariableType() &&
//     e_JavaVariableTypes::Array != reinterpret_cast<const IJavaVariableType *>(p)->GetVariableType()    )
  if ( nullptr == dynamic_cast<const ObjectReference *>(p) )
  {
    JVMXDetail::thread_safe_counter::increment( p->m_ref_counter );
  }
}

void intrusive_ptr_release( const JavaVariableTypeIntrusiveRefCounter* p ) JVMX_NOEXCEPT
{
  if ( nullptr == dynamic_cast<const ObjectReference *>( p ) )
//   if ( e_JavaVariableTypes::Object != reinterpret_cast<const IJavaVariableType *>(p)->GetVariableType() &&
//     e_JavaVariableTypes::Array != reinterpret_cast<const IJavaVariableType *>(p)->GetVariableType()
//     )
  {
    if ( JVMXDetail::thread_safe_counter::decrement( p->m_ref_counter ) == 0 )
    {
      delete reinterpret_cast<const IJavaVariableType *>(p);
    }
  }
}

unsigned int JVMXDetail::thread_safe_counter::load( boost::detail::atomic_count const& counter ) JVMX_NOEXCEPT
{
  return static_cast<unsigned int>(static_cast<long>(counter));
}

void JVMXDetail::thread_safe_counter::increment( boost::detail::atomic_count& counter ) JVMX_NOEXCEPT
{
  ++counter;
}

unsigned int JVMXDetail::thread_safe_counter::decrement( boost::detail::atomic_count& counter ) JVMX_NOEXCEPT
{
  return --counter;
}

JavaVariableTypeIntrusiveRefCounter& JavaVariableTypeIntrusiveRefCounter::operator=( JavaVariableTypeIntrusiveRefCounter const& ) JVMX_NOEXCEPT
{
  return *this;
}

JavaVariableTypeIntrusiveRefCounter::JavaVariableTypeIntrusiveRefCounter( JavaVariableTypeIntrusiveRefCounter const& ) JVMX_NOEXCEPT
  : m_ref_counter( 0 )
{

}

JavaVariableTypeIntrusiveRefCounter::JavaVariableTypeIntrusiveRefCounter() JVMX_NOEXCEPT
  : m_ref_counter( 0 )
{

}

unsigned int JavaVariableTypeIntrusiveRefCounter::use_count() const JVMX_NOEXCEPT
{
  return JVMXDetail::thread_safe_counter::load( m_ref_counter );
}

JavaVariableTypeIntrusiveRefCounter::~JavaVariableTypeIntrusiveRefCounter()
{

}
*/
