
#pragma once

#ifndef _OBJECTREFERENCE__H_
#define _OBJECTREFERENCE__H_

#include "ObjectRegistryLocalMachine.h"
#include "JavaTypes.h"

#include "jni_internal.h"

class ObjectReference : public IJavaVariableType
{
protected:
  friend class ObjectRegistryLocalMachine;

public:

  // Never call this explicitly. Only from the GC or the ObjectRegistry.
  explicit ObjectReference( ObjectIndexT index );

public:
  explicit ObjectReference( const jobject other );
  ObjectReference( const ObjectReference &other );
  const ObjectReference& operator=( const ObjectReference &other );

  virtual JavaObject *GetContainedObject() const { return GetObject(); }
  virtual JavaArray *GetContainedArray() const { return GetArray(); }

  virtual jobject ToJObject();
  virtual ObjectIndexT GetIndex() const;

#ifdef _DEBUG
  void AssertValid() const;
#endif // _DEBUG

public:
//   virtual void AddField( const JavaString &name, std::shared_ptr<FieldInfo> pFieldInfo );
//   virtual boost::intrusive_ptr<IJavaVariableType> GetFieldByName( const JavaString &name ) const;
//   virtual void SetField( const JavaString &name, boost::intrusive_ptr<IJavaVariableType> pValue );
// 
//   virtual boost::intrusive_ptr<IJavaVariableType> GetJVMXFieldByName( const JavaString &name ) const;
//   virtual void SetJVMXField( const JavaString &name, boost::intrusive_ptr<IJavaVariableType> pValue );
// 
//   virtual std::shared_ptr<JavaClass> GetClass();

  virtual void CloneOther( const ObjectReference &other );

public:
  virtual bool operator <( const ObjectReference &other ) const;
  virtual bool operator ==( const ObjectReference &other ) const;
  virtual bool operator !=( const ObjectReference &other ) const;

  virtual ObjectReference &operator=( const IJavaVariableType &other ) JVMX_OVERRIDE;

protected:
  virtual JavaObject *GetObject() const;
  virtual JavaArray *GetArray() const;

// IJavaVariableType methods
public:
  virtual e_JavaVariableTypes GetVariableType() const JVMX_OVERRIDE;
  virtual bool IsReferenceType() const JVMX_OVERRIDE;
  virtual bool IsIntegerCompatible() const JVMX_OVERRIDE;
  virtual bool IsNull() const JVMX_OVERRIDE;
  virtual JavaString ToString() const JVMX_OVERRIDE;

  virtual bool operator <( const IJavaVariableType & ) const JVMX_OVERRIDE;
  virtual bool operator ==( const IJavaVariableType & ) const JVMX_OVERRIDE;
  virtual bool operator !=( const IJavaVariableType &other ) const JVMX_OVERRIDE;

private:
  virtual IJavaVariableType *InternalGetObject( ObjectIndexT ref ) const;

private:
  ObjectIndexT m_Index;

#ifdef _DEBUG
  mutable IJavaVariableType const *m_pDebugPointer;
#endif // _DEBUG
};

#endif // _OBJECTREFERENCE__H_


