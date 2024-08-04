
#ifndef _BasicClassLibrary__H_
#define _BasicClassLibrary__H_

#include <unordered_map>

#include "JavaString.h"
#include "IClassLibrary.h"

class BasicClassLibrary : public IClassLibrary
{
public:
  virtual ~BasicClassLibrary();

  virtual void AddClass( std::shared_ptr<JavaClass> pClass ) JVMX_OVERRIDE;
  virtual std::shared_ptr<JavaClass> FindClass( const JavaString &className ) const JVMX_OVERRIDE;
  virtual std::shared_ptr<ConstantPoolEntry> GetConstant( const JavaString &className, size_t index ) const JVMX_OVERRIDE;

  virtual std::shared_ptr<MethodInfo> GetMethod( const JavaString &className, size_t index ) const JVMX_OVERRIDE;
  virtual std::shared_ptr<FieldInfo> GetField( const JavaString &className, size_t index ) const JVMX_OVERRIDE;
  virtual bool IsClassInitalised( const JavaString &className ) const JVMX_OVERRIDE;

  virtual std::vector<boost::intrusive_ptr<IJavaVariableType>> GetAllStaticObjectsAndArrays() const JVMX_OVERRIDE;

private:
  typedef std::unordered_map< JavaString, std::shared_ptr<JavaClass> > ClassContainer;

  mutable std::recursive_mutex m_Mutex;
  ClassContainer m_Classes;
};

#endif // _BasicClassLibrary__H_
