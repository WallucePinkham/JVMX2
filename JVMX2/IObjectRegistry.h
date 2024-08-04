#pragma once

#ifndef _IOBJECTREGISTRY__H_
#define _IOBJECTREGISTRY__H

#include "GlobalConstants.h"

#include <wallaroo/part.h>

#include "IIterator.h"
#include "IEnumerable.h"
#include "IJavaVariableType.h"

class JavaArray;
class JavaObject;

class ObjectReference;

typedef intptr_t ObjectIndexT;

class IObjectRegistry : public wallaroo::Part, public IEnumerable
{
public:
  friend class ObjectReference;

public:
  virtual ~IObjectRegistry() JVMX_NOEXCEPT {};

public:
  virtual size_t GetCount() const JVMX_OVERRIDE JVMX_PURE;
  
  virtual std::shared_ptr<const IIterator> GetFirst() const JVMX_OVERRIDE JVMX_PURE;
  virtual bool HasMore( const std::shared_ptr<const IIterator> &it ) const JVMX_OVERRIDE JVMX_PURE;
  virtual std::shared_ptr<const IIterator> GetNext( const std::shared_ptr<const IIterator> &it ) const JVMX_OVERRIDE JVMX_PURE;

  virtual ObjectIndexT GetIndexAt( const std::shared_ptr<const IIterator> &it ) const JVMX_PURE;
  virtual bool HasBeenUpdatedAt( const std::shared_ptr<const IIterator> &it ) const JVMX_PURE;

public:
  
  virtual ObjectReference AddObject( JavaObject *pObject ) JVMX_PURE;
  virtual ObjectReference AddObject( JavaArray *pArray ) JVMX_PURE;

  // For use by the Garbage Collector
  virtual void RemoveObject( ObjectIndexT ref ) JVMX_PURE;
  virtual void UpdateObjectPointer( const ObjectReference &ref, IJavaVariableType *pObject ) JVMX_PURE;

  virtual void Cleanup() JVMX_PURE;

  virtual void VerifyEntry( ObjectIndexT ref ) JVMX_PURE;

protected:
  virtual IJavaVariableType *GetObject_( ObjectIndexT ref ) JVMX_PURE;
};

#endif // _IOBJECTREGISTRY__H_
