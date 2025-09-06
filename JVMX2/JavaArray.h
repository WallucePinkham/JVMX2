#pragma once

#ifndef _JAVAARRAY__H_
#define _JAVAARRAY__H_

#include <vector>
#include <mutex>

#include <boost/intrusive_ptr.hpp>

#include "GlobalConstants.h"
#include "IJavaVariableType.h"
#include "JavaArrayTypes.h"
#include "MemoryAllocator.h"
#include "Lockable.h"

class JavaInteger;
class JavaChar;
class JavaByte;
class JavaBool;
class JavaShort;
class JavaLong;
class ObjectReference;

class JavaArray : protected IJavaVariableType
{
  // Not really happy with this. There should be a single friend class that is allowed to create JavaArray instances.
  friend class BasicVirtualMachineState;
  friend class HelperTypes;
  friend class CheneyGarbageCollector;

private:
  JavaArray() JVMX_NOEXCEPT {};

public:
  // NOTE: This is expected to be created via placement new.
  explicit JavaArray( e_JavaArrayTypes type, size_t size );

  static size_t CalculateSizeInBytes( e_JavaArrayTypes type, size_t count );
  static size_t GetSizeOfValueType( e_JavaArrayTypes type );

  //JavaArray( const JavaArray &other );

  virtual ~JavaArray() JVMX_NOEXCEPT;

  void operator delete ( void *pObject ) throw();
  void operator delete ( void *pObject, void * ) throw();

  virtual e_JavaVariableTypes GetVariableType() const JVMX_OVERRIDE;
  virtual bool IsReferenceType() const JVMX_OVERRIDE;
  virtual bool IsIntegerCompatible() const JVMX_OVERRIDE;
  virtual bool IsNull() const JVMX_OVERRIDE;
  virtual JavaString ToString() const JVMX_OVERRIDE;

  e_JavaArrayTypes GetContainedType() const;

  virtual bool operator==( const IJavaVariableType &other ) const JVMX_OVERRIDE;
  virtual bool operator==( const JavaArray &other ) const;

  virtual bool operator<( const IJavaVariableType &other ) const JVMX_OVERRIDE;
  virtual bool operator<( const JavaArray &other ) const;

  static e_JavaArrayTypes ConvertTypeFromChar( char16_t charType );

  static boost::intrusive_ptr<ObjectReference> CreateFromCArray( /*std::shared_ptr<IMemoryManager> pMemoryManager,*/ const char *pBuffer );
  static boost::intrusive_ptr<ObjectReference> CreateFromCArray( /*std::shared_ptr<IMemoryManager> pMemoryManager,*/ const uint8_t* pBuffer, size_t length);

  virtual JavaChar CharAt(size_t index) const;
  virtual JavaByte ByteAt(size_t index) const;
  virtual JavaBool BoolAt(size_t index) const;
  virtual JavaFloat FloatAt(size_t index) const;
  virtual JavaDouble DoubleAt(size_t index) const;
  virtual JavaInteger IntegerAt(size_t index) const;
  virtual JavaShort ShortAt(size_t index) const;
  virtual JavaLong LongAt(size_t index) const;

  virtual IJavaVariableType *At( size_t index );
  virtual const IJavaVariableType *At( size_t index ) const;
  virtual size_t GetNumberOfElements() const;

  void SetAt( const JavaInteger &index, const JavaInteger &value );
  void SetAt( const JavaInteger &index, const JavaChar &value );
  void SetAt( const JavaInteger &index, const IJavaVariableType *pValue );
  void SetAt(const JavaInteger& index, const JavaByte& value);
  void SetAt(const JavaInteger& index, const JavaBool& value);
  void SetAt(const JavaInteger& index, JavaLong value);
  void SetAt(const JavaInteger& index, const JavaFloat& value);
  void SetAt(const JavaInteger& index, const JavaDouble& value);
  void SetAt(const JavaInteger& index, const JavaShort& value);

  void SetAt( const uint32_t &index, const JavaInteger &value );
  void SetAt( const uint32_t &index, const JavaChar &value );
  void SetAt( const uint32_t &index, const IJavaVariableType *pValue );
  void SetAt(const uint32_t& index, const JavaByte& value);
  void SetAt(const uint32_t& index, const JavaBool& value);
  void SetAt(const uint32_t& index, const JavaFloat& value);
  void SetAt(const uint32_t& index, const JavaDouble& value);
  void SetAt(const uint32_t& index, JavaLong value);
  void SetAt(const uint32_t& index, JavaShort value);

  JavaString ConvertCharArrayToString() const;
  DataBuffer ConvertByteArrayToBuffer() const;
  JavaString ConvertByteArrayToString() const;

  boost::intrusive_ptr<IJavaVariableType> ConvertIntegerTypeForArrayStorage( const JavaInteger &value ) const;
  //boost::intrusive_ptr<IJavaVariableType> ConvertReferenceTypeForArrayStorage( const IJavaVariableType *pValue ) const;

  std::shared_ptr<Lockable> MonitorEnter( const char *pFunctionName );
  void MonitorExit( const char *pFunctionName );

  void CloneOther( const JavaArray *pObjectToClone );

  // ONLY TO BE USED FOR GARBAGE COLLECTION
  void DeepClone( const JavaArray *pObjectToClone );

private:
  void Initialise();

  IJavaVariableType *GetValueAtIndex( size_t i );
  const IJavaVariableType *GetValueAtIndex( size_t i ) const;

  void DebugAssert() const;
  void InternalSetValue( const size_t index, const IJavaVariableType *pFinalValue );
  inline void ValidateIndex(const uint32_t& index) const;



  //   void SetAt( const uint32_t &index, IJavaVariableType *pValue ) JVMX_FN_DELETE;
  //   void SetAt( const JavaInteger &index, IJavaVariableType *pValue ) JVMX_FN_DELETE;

private:
  static bool AreTypesCompatible( e_JavaArrayTypes arrayType, e_JavaVariableTypes variableType );
  static bool AreTypesCompatible( e_JavaArrayTypes arrayType, e_JavaArrayTypes variableType );

  static bool IsTypeIntegerCompatible( e_JavaArrayTypes variableType );
  static bool IsTypeIntegerCompatible( e_JavaVariableTypes variableType );

  static bool IsVariableOfReferenceType( e_JavaVariableTypes variableType );



private:
  e_JavaArrayTypes m_ContainedType = e_JavaArrayTypes::Reference;
  size_t m_Size = 0;

private:
  std::shared_ptr<Lockable> m_pMonitor;

#ifdef _DEBUG
  size_t debugInitialLength = 0;
#endif // _DEBUG

private:

  char m_pValues[1] = { 0 };
};

#endif // _JAVAARRAY__H_
