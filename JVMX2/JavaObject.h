#ifndef _JAVAOBJECT__H_
#define _JAVAOBJECT__H_

#include <unordered_map>
#include <memory>
#include <mutex>
#include <condition_variable>

//#include "OsFunctions.h"

#include "GlobalConstants.h"
#include "IJavaVariableType.h"
//#include "VariableComparison.h"
#include "JavaString.h"
#include "MemoryAllocator.h"
#include "Lockable.h"

class JavaClass; // Forward declaration
class FieldInfo; // Forward declaration
class ObjectFactory;
class IMemoryManager;

class JavaObject : protected IJavaVariableType
{
  friend class BasicVirtualMachineState;
  friend class ObjectReference;

public:
  explicit JavaObject( std::shared_ptr<JavaClass> pClass );

public:

  virtual ~JavaObject() JVMX_NOEXCEPT;

  JavaObject &operator=( const JavaObject &other );

  void operator delete ( void *pObject ) throw();
  void operator delete ( void *pObject, void * ) throw();

  virtual e_JavaVariableTypes GetVariableType() const JVMX_OVERRIDE;
  virtual bool IsReferenceType() const JVMX_OVERRIDE;
  virtual bool IsIntegerCompatible() const JVMX_OVERRIDE;
  virtual bool IsNull() const JVMX_OVERRIDE;
  virtual JavaString ToString() const JVMX_OVERRIDE;

  virtual bool operator==( const IJavaVariableType &other ) const JVMX_OVERRIDE;
  virtual bool operator==( const JavaObject &other ) const;

  virtual bool operator<( const IJavaVariableType &other ) const  JVMX_OVERRIDE;
  virtual bool operator<( const JavaObject &other ) const;

  virtual int CompareFields( const JavaObject &other ) const;

  //virtual void AddField( const JavaString &name, std::shared_ptr<FieldInfo> pFieldInfo );
  virtual boost::intrusive_ptr<IJavaVariableType> GetFieldByName( const JavaString &name );
  std::shared_ptr<FieldInfo>  ResolveField( const JavaString &name, size_t &fieldOffset ) const;
  virtual boost::intrusive_ptr<IJavaVariableType> GetFieldByNameConst( const JavaString &name ) const;

  virtual void SetField( const JavaString &name, boost::intrusive_ptr<IJavaVariableType> pValue, bool allowNonPublic = true );
  virtual void SetField( const JavaString &name, IJavaVariableType *pValue, bool allowNonPublic = true );

  virtual boost::intrusive_ptr<IJavaVariableType> GetJVMXFieldByName( const JavaString &name ) const;
  virtual void SetJVMXField( const JavaString &name, boost::intrusive_ptr<IJavaVariableType> pValue );

  virtual std::shared_ptr<JavaClass> GetClass() const;

  virtual std::shared_ptr<Lockable> MonitorEnter( const char *pFuctionName );
  virtual void MonitorExit( const char *pFuctionName );

  void CloneOther( const JavaObject *pObjectToClone );

  void Wait( JavaLong milliSeconds, JavaInteger nanoSeconds );
  void NotifyOne();
  void NotifyAll();

  virtual bool IsInstanceOf( const JavaString &pPossibleSuperClassName ) const;

  size_t GetSizeInBytes() const;

  // ONLY TO BE USED FOR GARBAGE COLLECTION
  void DeepClone( const JavaObject *pObjectToClone );

private:
  void AssertValid() const;

  void CloneOther( boost::intrusive_ptr<JavaObject> pObjectToClone ) JVMX_FN_DELETE;

  JavaObject( const JavaObject &other ) JVMX_FN_DELETE;
  JavaObject( JavaObject &&other ) JVMX_FN_DELETE;

  size_t InitialiseFields( const std::shared_ptr<JavaClass> &pClass );
  size_t InitialiseField( std::shared_ptr<JavaClass> pClass,  int i, size_t startingOffset );

  bool ThrowJavaExceptionIfInterrupted() const;

private:
  std::shared_ptr<JavaClass> m_pClass;

  std::unordered_map < JavaString, boost::intrusive_ptr<IJavaVariableType>, std::hash<JavaString>, std::equal_to<JavaString>> m_JVMXFields;

private:
  std::shared_ptr<Lockable> m_pMonitor;
  std::shared_ptr<std::condition_variable_any> m_Waitable;

  volatile bool m_Notfied; // To protect against spurious wake-ups.

#ifdef _DEBUG
  std::vector<std::pair<JavaString, size_t>> m_DebugList;
#endif // _DEBUG


  // NB that this HAS to be the last field!
  char m_pFields[1];
};

#endif // _JAVAOBJECT__H_
