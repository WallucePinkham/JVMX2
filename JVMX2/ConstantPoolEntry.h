
#pragma once

#ifndef __CONSTANTPOOLENTRY_H__
#define __CONSTANTPOOLENTRY_H__

#include <memory>

#include "GlobalConstants.h"

#include "IConstantPoolEntryValue.h"
#include "ConstantPoolClassReference.h"
#include "ConstantPoolStringReference.h"
#include "ConstantPoolDualReference.h"
#include "ConstantPoolFieldReference.h"
#include "ConstantPoolMethodReference.h"
#include "ConstantPoolInterfaceMethodReference.h"
#include "ConstantPoolNameAndTypeDescriptor.h"
#include "ConstantPoolNullEntry.h"

#include "ObjectReference.h"

enum e_ConstantPoolEntryTypes : uint8_t
{
  e_ConstantPoolEntryTypeNullEntry = UINT8_MAX,
  e_ConstantPoolEntryTypeString = 1,
  e_ConstantPoolEntryTypeInteger = 3,
  e_ConstantPoolEntryTypeFloat = 4,
  e_ConstantPoolEntryTypeLong = 5,
  e_ConstantPoolEntryTypeDouble = 6,
  e_ConstantPoolEntryTypeClassReference = 7,
  e_ConstantPoolEntryTypeStringReference = 8,
  e_ConstantPoolEntryTypeFieldReference = 9,
  e_ConstantPoolEntryTypeMethodReference = 10,
  e_ConstantPoolEntryTypeInterfaceMethodReference = 11,
  e_ConstantPoolEntryTypeNameAndTypeDescriptor = 12,
  e_ConstantPoolEntryTypeMethodHandle = 15,
  e_ConstantPoolEntryTypeMethodType = 16,
  e_ConstantPoolEntryTypeInvokeDynamicInfo = 18,
};

class IJavaVariableType;
class JavaArray;

class ConstantPoolEntry
{
public:
  explicit ConstantPoolEntry( const JavaString &string );
  explicit ConstantPoolEntry( JavaString &&string );
  explicit ConstantPoolEntry( JavaInteger intValue );
  explicit ConstantPoolEntry( JavaLong longValue );
  explicit ConstantPoolEntry( JavaFloat floatValue );
  explicit ConstantPoolEntry( JavaDouble doubleValue );
  explicit ConstantPoolEntry( std::shared_ptr<ConstantPoolClassReference> pRef );
  explicit ConstantPoolEntry( std::shared_ptr<ConstantPoolStringReference> pRef );
  explicit ConstantPoolEntry( std::shared_ptr<ConstantPoolFieldReference> pRef );
  explicit ConstantPoolEntry( std::shared_ptr<ConstantPoolMethodReference> pRef );
  explicit ConstantPoolEntry( std::shared_ptr<ConstantPoolInterfaceMethodReference> pRef );
  explicit ConstantPoolEntry( std::shared_ptr<ConstantPoolNameAndTypeDescriptor> pRef );
  explicit ConstantPoolEntry( ConstantPoolNullEntry entry );

  virtual ~ConstantPoolEntry() JVMX_NOEXCEPT;

  ConstantPoolEntry( const ConstantPoolEntry &other );
  ConstantPoolEntry( ConstantPoolEntry &&other ) JVMX_NOEXCEPT;

  ConstantPoolEntry& operator=(ConstantPoolEntry other) JVMX_NOEXCEPT;

  virtual e_ConstantPoolEntryTypes GetType() const JVMX_NOEXCEPT;
  virtual const IConstantPoolEntryValue *GetValue() const JVMX_NOEXCEPT;

  std::shared_ptr<ConstantPoolClassReference> AsClassReferencePointer();
  std::shared_ptr<ConstantPoolStringReference> AsStringReference();
  std::shared_ptr<ConstantPoolMethodReference> AsMethodReference();

  //__declspec(deprecated) ConstantPoolClassReference GetClassReference() const;
  boost::intrusive_ptr<JavaString> AsString() const;
  boost::intrusive_ptr<ObjectReference> AsArrayOfChars() const;

  boost::intrusive_ptr<JavaInteger> AsInteger() const;
  boost::intrusive_ptr<JavaFloat> AsFloat() const;

  boost::intrusive_ptr<JavaLong> AsLong() const;
  boost::intrusive_ptr<JavaDouble> AsDouble() const;

  //__declspec(deprecated) ConstantPoolNameAndTypeDescriptor GetNameAndTypeDescriptor() const;
  std::shared_ptr<ConstantPoolNameAndTypeDescriptor> AsNameAndTypeDescriptorPointer();

  std::shared_ptr<ConstantPoolFieldReference> AsFieldReferencePointer() const;

  boost::intrusive_ptr<IJavaVariableType> AsJavaVariable() const;

  void Prepare( const ConstantPool *pPool );

// #ifdef _DEBUG
// public:
//   void DebugDump();
// #endif // _DEBUG

private:
  // In order to conform with the general C++ standards, this method name is intentionally all lower case.
  void swap( ConstantPoolEntry &left, ConstantPoolEntry &right ) JVMX_NOEXCEPT;

private:
  e_ConstantPoolEntryTypes m_Type;
  std::shared_ptr<IConstantPoolEntryValue> m_pValue;
};

#endif // __CONSTANTPOOLENTRY_H__
