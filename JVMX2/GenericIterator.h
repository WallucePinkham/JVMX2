#pragma once

#ifndef _GENERICITERATOR__H_
#define  _GENERICITERATOR__H_

#include "IIterator.h"

template<typename T>
class GenericIterator : public IIterator
{
public:
  virtual ~GenericIterator() JVMX_NOEXCEPT
  {
  }

  explicit GenericIterator( typename T::const_iterator it )
    : m_Iterator( it )
  {
  }

  virtual std::shared_ptr<const IIterator> GetNext() const JVMX_OVERRIDE
  {
    typename T::const_iterator newIterator = m_Iterator;
    return std::make_shared<const GenericIterator<T>>( ++newIterator );
  }

  virtual std::shared_ptr<const IIterator> GetPrevious() const JVMX_OVERRIDE
  {
    typename T::const_iterator newIterator = m_Iterator;
    return std::make_shared<GenericIterator<T>>( --newIterator );
  }

  virtual bool IsEnd( const T &container) const
  {
    return m_Iterator == container.cend();
  }

  virtual typename T::value_type Dereference() const
  {
    return *m_Iterator;
  }

private:
  typename T::const_iterator m_Iterator;
};

#endif // _GENERICITERATOR__H_
