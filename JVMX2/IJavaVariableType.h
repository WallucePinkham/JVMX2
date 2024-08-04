
#ifndef __IJAVAVARIABLETYPE_H__
#define __IJAVAVARIABLETYPE_H__

#include <boost/smart_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>

#include "GlobalConstants.h"
#include "JavaVariableTypeIntrusiveRefCounter.h"

enum class e_JavaVariableTypes : uint16_t
{
  Char
  , Byte
  , Short
  , Integer
  , Long
  , Float
  , Double
  , String
  , NullReference
  , ReturnAddress
  , ClassReference
  , Bool
  , Array
  , Object
};

class JavaString;

class IJavaVariableType /*JVMX_ABSTRACT*/ : public boost::intrusive_ref_counter<IJavaVariableType>
{
public:
  virtual ~IJavaVariableType() JVMX_NOEXCEPT;

  virtual e_JavaVariableTypes GetVariableType() const JVMX_PURE;

  virtual bool IsReferenceType() const JVMX_PURE;
  virtual bool IsIntegerCompatible() const JVMX_PURE;
  virtual bool IsNull() const JVMX_PURE;
  virtual JavaString ToString() const JVMX_PURE;

  virtual bool operator<(const IJavaVariableType &) const;
  virtual bool operator==(const IJavaVariableType &) const;
  virtual bool operator!=(const IJavaVariableType &other) const;

  virtual IJavaVariableType &operator=( const IJavaVariableType &other );
};



#endif // __IJAVAVARIABLETYPE_H__
