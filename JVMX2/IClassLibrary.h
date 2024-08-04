
#ifndef _ICLASSLIBRARY__H_
#define _ICLASSLIBRARY__H_

#include <memory>
#include <wallaroo/part.h>

#include "JavaClass.h"

#include "ConstantPool.h"

class ConstantPoolEntry;

class IClassLibrary JVMX_ABSTRACT : public wallaroo::Part
{
public:
  virtual ~IClassLibrary() {};

  virtual void AddClass( std::shared_ptr<JavaClass> pClass ) JVMX_PURE;
  virtual std::shared_ptr<JavaClass> FindClass( const JavaString &className ) const JVMX_PURE;

  virtual std::shared_ptr<ConstantPoolEntry> GetConstant( const JavaString &className, size_t index ) const JVMX_PURE;
  virtual std::shared_ptr<MethodInfo> GetMethod( const JavaString &className, size_t index ) const JVMX_PURE;
  virtual std::shared_ptr<FieldInfo> GetField( const JavaString &className, size_t index ) const JVMX_PURE;
  //virtual FieldInfo GetField( JavaString className, size_t index ) const JVMX_PURE;
  virtual bool IsClassInitalised( const JavaString &className ) const JVMX_PURE;

  virtual std::vector<boost::intrusive_ptr<IJavaVariableType>> GetAllStaticObjectsAndArrays() const JVMX_PURE;

protected:
  IClassLibrary() {}
};

#endif // _ICLASSLIBRARY__H_
