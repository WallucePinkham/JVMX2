/*

// IJavaVariableTypeIntrusiveRefCounter
#ifndef _IJAVAVARIABLETYPEINTRUSIVEREFCOUNTER__H_
#define _IJAVAVARIABLETYPEINTRUSIVEREFCOUNTER__H_

#include "GlobalConstants.h"

#include <boost/smart_ptr/detail/atomic_count.hpp>

class IJavaVariableType;

namespace JVMXDetail
{
  / *!
  * \brief Thread safe reference counter policy for \c intrusive_ref_counter
  *
  * The policy instructs the \c intrusive_ref_counter base class to implement
  * a thread-safe reference counter, if the target platform supports multithreading.
  * /
  struct thread_safe_counter
  {
    typedef boost::detail::atomic_count type;

    static unsigned int load( boost::detail::atomic_count const& counter ) JVMX_NOEXCEPT;

    static void increment( boost::detail::atomic_count& counter ) JVMX_NOEXCEPT;

    static unsigned int decrement( boost::detail::atomic_count& counter ) JVMX_NOEXCEPT;
  };
};

class JavaVariableTypeIntrusiveRefCounter;

void intrusive_ptr_add_ref( const JavaVariableTypeIntrusiveRefCounter* p ) JVMX_NOEXCEPT;
void intrusive_ptr_release( const JavaVariableTypeIntrusiveRefCounter* p ) JVMX_NOEXCEPT;

class JavaVariableTypeIntrusiveRefCounter
{
private:
  //! Reference counter type
  typedef typename JVMXDetail::thread_safe_counter::type counter_type;
  //! Reference counter
  mutable counter_type m_ref_counter;

public:
  / *!
  * Default constructor
  *
  * \post <tt>use_count() == 0</tt>
  * /
  JavaVariableTypeIntrusiveRefCounter() JVMX_NOEXCEPT;

  / *!
  * Copy constructor
  *
  * \post <tt>use_count() == 0</tt>
  * /
  JavaVariableTypeIntrusiveRefCounter( JavaVariableTypeIntrusiveRefCounter const& ) JVMX_NOEXCEPT;

  / *!
  * Assignment
  *
  * \post The reference counter is not modified after assignment
  * /
  JavaVariableTypeIntrusiveRefCounter& operator= ( JavaVariableTypeIntrusiveRefCounter const& ) JVMX_NOEXCEPT;

  / *!
  * \return The reference counter
  * /
  unsigned int use_count() const JVMX_NOEXCEPT;

protected:
  / *!
  * Destructor
  * /
  virtual ~JavaVariableTypeIntrusiveRefCounter();

  friend void intrusive_ptr_add_ref( const JavaVariableTypeIntrusiveRefCounter* p ) JVMX_NOEXCEPT;
  friend void intrusive_ptr_release( const JavaVariableTypeIntrusiveRefCounter* p ) JVMX_NOEXCEPT;
};



#endif // _IJAVAVARIABLETYPEINTRUSIVEREFCOUNTER__H_


*/
