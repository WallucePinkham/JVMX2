#pragma once

#ifndef _IENUMERABLE__H_
#define _IENUMERABLE__H_

#include "GlobalConstants.h"

#include <memory>

class IIterator;

class IEnumerable
{
public:
  virtual ~IEnumerable() JVMX_NOEXCEPT {};

  virtual size_t GetCount() const JVMX_PURE;

  virtual std::shared_ptr<const IIterator> GetFirst() const JVMX_PURE;
  virtual bool HasMore( const std::shared_ptr<const IIterator> &it ) const JVMX_PURE;
  virtual std::shared_ptr<const IIterator> GetNext( const std::shared_ptr<const IIterator> &it ) const JVMX_PURE;
};

#endif // _IENUMERABLE__H_
