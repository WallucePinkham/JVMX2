#pragma once

#ifndef _IITERATOR__H_
#define _IITERATOR__H_

#include "GlobalConstants.h"

class IIterator
{
public:
  virtual ~IIterator() JVMX_NOEXCEPT {};

  virtual std::shared_ptr<const IIterator> GetNext() const JVMX_PURE;
  virtual std::shared_ptr<const IIterator> GetPrevious() const JVMX_PURE;
};

#endif // _IITERATOR__H_
