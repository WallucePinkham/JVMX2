#include <memory>

#include "JavaTypes.h"

#include "TypeMismatchException.h"
#include "IThreadManager.h"
#include "IVirtualMachineState.h"
#include "GlobalCatalog.h"

//#include "OsFunctions.h"

#include "ConstantPoolEntry.h"



ConstantPoolEntry::ConstantPoolEntry( const ConstantPoolEntry &other )
{
  m_pValue = other.m_pValue;
  m_Type = other.m_Type;
}

ConstantPoolEntry::ConstantPoolEntry( ConstantPoolEntry &&other ) JVMX_NOEXCEPT
{
  m_Type = e_ConstantPoolEntryTypeString;
  m_pValue = nullptr;

  swap( *this, other );
}

ConstantPoolEntry::ConstantPoolEntry( const JavaString &string )
{
  m_Type = e_ConstantPoolEntryTypeString;
  m_pValue = std::make_shared<JavaString>( string );
}

ConstantPoolEntry::ConstantPoolEntry( JavaString &&string )
{
  m_Type = e_ConstantPoolEntryTypeString;
  m_pValue = std::make_shared<JavaString>( std::move( string ) );
}

ConstantPoolEntry::ConstantPoolEntry( JavaInteger intValue )
{
  m_Type = e_ConstantPoolEntryTypeInteger;
  m_pValue = std::make_shared<JavaInteger>( intValue );
}

ConstantPoolEntry::ConstantPoolEntry( JavaLong longValue )
{
  m_Type = e_ConstantPoolEntryTypeLong;
  m_pValue = std::make_shared<JavaLong>( longValue );
}

ConstantPoolEntry::ConstantPoolEntry( JavaFloat floatValue )
{
  m_Type = e_ConstantPoolEntryTypeFloat;
  m_pValue = std::make_shared<JavaFloat>( floatValue );
}

ConstantPoolEntry::ConstantPoolEntry( JavaDouble doubleValue )
{
  m_Type = e_ConstantPoolEntryTypeDouble;
  m_pValue = std::make_shared<JavaDouble>( doubleValue );
}

ConstantPoolEntry::ConstantPoolEntry( std::shared_ptr<ConstantPoolClassReference> pRef )
{
  m_Type = e_ConstantPoolEntryTypeClassReference;
  m_pValue = pRef;
}

ConstantPoolEntry::ConstantPoolEntry( std::shared_ptr<ConstantPoolStringReference> pRef )
{
  m_Type = e_ConstantPoolEntryTypeStringReference;
  m_pValue = pRef;
}

ConstantPoolEntry::ConstantPoolEntry( std::shared_ptr<ConstantPoolFieldReference> pRef )
{
  m_Type = e_ConstantPoolEntryTypeFieldReference;
  m_pValue = pRef;
}

ConstantPoolEntry::ConstantPoolEntry( std::shared_ptr<ConstantPoolMethodReference> pRef )
{
  m_Type = e_ConstantPoolEntryTypeMethodReference;
  m_pValue = pRef;
}

ConstantPoolEntry::ConstantPoolEntry( std::shared_ptr<ConstantPoolInterfaceMethodReference> pRef )
{
  m_Type = e_ConstantPoolEntryTypeInterfaceMethodReference;
  m_pValue = pRef;
}

ConstantPoolEntry::ConstantPoolEntry( std::shared_ptr<ConstantPoolNameAndTypeDescriptor> pRef )
{
  m_Type = e_ConstantPoolEntryTypeNameAndTypeDescriptor;
  m_pValue = pRef;
}

ConstantPoolEntry::ConstantPoolEntry(std::shared_ptr<ConstantPoolInvokeDynamic> pRef)
{
  m_Type = e_ConstantPoolEntryTypeInvokeDynamicInfo;
  m_pValue = pRef;
}

ConstantPoolEntry::ConstantPoolEntry(std::shared_ptr<ConstantPoolMethodHandle> pRef)
{
  m_Type = e_ConstantPoolEntryTypeMethodHandle;
  m_pValue = pRef;
}

ConstantPoolEntry::ConstantPoolEntry( ConstantPoolNullEntry entry )
{
  m_Type = e_ConstantPoolEntryTypeNullEntry;
  m_pValue = nullptr;
}

ConstantPoolEntry::~ConstantPoolEntry()
{}

e_ConstantPoolEntryTypes ConstantPoolEntry::GetType() const JVMX_NOEXCEPT
{
  return m_Type;
}

ConstantPoolEntry &ConstantPoolEntry::operator=( ConstantPoolEntry other ) JVMX_NOEXCEPT
{
  swap( *this, other );

  return *this;
}

const IConstantPoolEntryValue *ConstantPoolEntry::GetValue() const JVMX_NOEXCEPT
{
  return m_pValue.get();
}

void ConstantPoolEntry::swap( ConstantPoolEntry &left, ConstantPoolEntry &right ) JVMX_NOEXCEPT
{
  std::swap( left.m_pValue, right.m_pValue );
  std::swap( left.m_Type, right.m_Type );
}

// ConstantPoolClassReference ConstantPoolEntry::GetClassReference() const
// {
//   if ( GetType() != e_ConstantPoolEntryTypeClassReference )
//   {
//     throw TypeMismatchException( __FUNCTION__ " - Constant pool entry is not a class reference as expected." );
//   }
//
//   return *(dynamic_cast<const ConstantPoolClassReference *>(GetValue()));
// }

boost::intrusive_ptr<JavaString> ConstantPoolEntry::AsString() const
{
  if ( GetType() != e_ConstantPoolEntryTypeString )
  {
    throw TypeMismatchException( __FUNCTION__ " - Constant pool entry is not a string as expected." );
  }

  return new JavaString( *std::dynamic_pointer_cast<JavaString>( m_pValue ) );
}

// ConstantPoolNameAndTypeDescriptor ConstantPoolEntry::GetNameAndTypeDescriptor() const
// {
//   if ( GetType() != e_ConstantPoolEntryTypeNameAndTypeDescriptor )
//   {
//     throw TypeMismatchException( __FUNCTION__ " - Constant pool entry is not a named and type descriptor as expected." );
//   }
//
//   return *(dynamic_cast<const ConstantPoolNameAndTypeDescriptor *>(GetValue()));
// }

std::shared_ptr<ConstantPoolClassReference> ConstantPoolEntry::AsClassReferencePointer()
{
  if ( GetType() != e_ConstantPoolEntryTypeClassReference )
  {
    throw TypeMismatchException( __FUNCTION__ " - Constant pool entry is not a class reference as expected." );
  }

  return std::dynamic_pointer_cast<ConstantPoolClassReference>( m_pValue );
}

std::shared_ptr<ConstantPoolNameAndTypeDescriptor> ConstantPoolEntry::AsNameAndTypeDescriptorPointer()
{
  if ( GetType() != e_ConstantPoolEntryTypeNameAndTypeDescriptor )
  {
    throw TypeMismatchException( __FUNCTION__ " - Constant pool entry is not a name and type descriptor as expected." );
  }

  return std::dynamic_pointer_cast<ConstantPoolNameAndTypeDescriptor>( m_pValue );
}

void ConstantPoolEntry::Prepare( const ConstantPool *pPool )
{
  if ( nullptr != m_pValue )
  {
    m_pValue->Prepare( pPool );
  }
}

// #ifdef _DEBUG
// void ConstantPoolEntry::DebugDump()
// {
//   FILE *pFile = fopen( "ConstantPool.txt", "a+" );
//
//   if ( nullptr != pFile )
//   {
//     if ( m_Type == e_ConstantPoolEntryTypeClassReference )
//     {
//       fprintf( pFile, "Entry:\n\tClass Reference (%s)\n", AsClassReferencePointer()->GetClassName()->ToByteArray() );
//     }
//     else if ( m_Type == e_ConstantPoolEntryTypeFieldReference )
//     {
//       fprintf( pFile, "Entry:\n\tField Reference (%s %s::%s)\n", AsFieldReferencePointer()->GetType()->ToByteArray(), AsFieldReferencePointer()->GetClassName()->ToByteArray(), AsFieldReferencePointer()->GetName()->ToByteArray() );
//     }
//     else if ( m_Type == e_ConstantPoolEntryTypeMethodReference )
//     {
//       fprintf( pFile, "Entry:\n\tMethod Reference (%s %s::%s)\n", AsMethodReference()->GetType()->ToByteArray(), AsMethodReference()->GetClassName()->ToByteArray(), AsMethodReference()->GetName()->ToByteArray() );
//     }
//
//     fclose( pFile );
//   }
// }
// #endif // _DEBUG

std::shared_ptr<ConstantPoolFieldReference> ConstantPoolEntry::AsFieldReferencePointer() const
{
  if ( GetType() != e_ConstantPoolEntryTypeFieldReference )
  {
    throw TypeMismatchException( __FUNCTION__ " - Constant pool entry is not a field reference as expected." );
  }

  return std::dynamic_pointer_cast<ConstantPoolFieldReference>( m_pValue );
}

boost::intrusive_ptr< JavaInteger > ConstantPoolEntry::AsInteger() const
{
  if ( GetType() != e_ConstantPoolEntryTypeInteger )
  {
    throw TypeMismatchException( __FUNCTION__ " - Constant pool entry is not an integer as expected." );
  }

  return new JavaInteger( *std::dynamic_pointer_cast<JavaInteger>( m_pValue ) );
}

boost::intrusive_ptr<JavaFloat> ConstantPoolEntry::AsFloat() const
{
  if ( GetType() != e_ConstantPoolEntryTypeFloat )
  {
    throw TypeMismatchException( __FUNCTION__ " - Constant pool entry is not an integer as expected." );
  }

  return new JavaFloat( *std::dynamic_pointer_cast<JavaFloat>( m_pValue ) );
}

boost::intrusive_ptr<JavaLong> ConstantPoolEntry::AsLong() const
{
  if ( GetType() != e_ConstantPoolEntryTypeLong )
  {
    throw TypeMismatchException( __FUNCTION__ " - Constant pool entry is not an long as expected." );
  }

  return new JavaLong( *std::dynamic_pointer_cast<JavaLong>( m_pValue ) );
}

boost::intrusive_ptr<JavaDouble> ConstantPoolEntry::AsDouble() const
{
  if ( GetType() != e_ConstantPoolEntryTypeDouble )
  {
    throw TypeMismatchException( __FUNCTION__ " - Constant pool entry is not an double as expected." );
  }

  return new JavaDouble( *std::dynamic_pointer_cast<JavaDouble>( m_pValue ) );
}

std::shared_ptr<ConstantPoolStringReference> ConstantPoolEntry::AsStringReference()
{
  if ( GetType() != e_ConstantPoolEntryTypeStringReference )
  {
    throw TypeMismatchException( __FUNCTION__ " - Constant pool entry is not a string reference as expected." );
  }

  return std::dynamic_pointer_cast<ConstantPoolStringReference>( m_pValue );
}

std::shared_ptr<ConstantPoolMethodReference> ConstantPoolEntry::AsMethodReference()
{
  if ( GetType() != e_ConstantPoolEntryTypeMethodReference && GetType() != e_ConstantPoolEntryTypeInterfaceMethodReference )
  {
    throw TypeMismatchException( __FUNCTION__ " - Constant pool entry is not a method reference as expected." );
  }

  return std::dynamic_pointer_cast<ConstantPoolMethodReference>( m_pValue );
}

boost::intrusive_ptr<IJavaVariableType> ConstantPoolEntry::AsJavaVariable() const
{
  boost::intrusive_ptr<IJavaVariableType> pResult = dynamic_cast<IJavaVariableType *>( m_pValue.get() );
  if ( nullptr == pResult )
  {
    if ( GetType() == e_ConstantPoolEntryTypeStringReference )
    {
      auto pStringRef = dynamic_cast<ConstantPoolStringReference *>( m_pValue.get() );
      pResult = pStringRef->GetStringValue();
    }
    else
    {
      /// Still to be implemented.
      JVMX_ASSERT( false );
    }
  }

  return pResult;
}

boost::intrusive_ptr<ObjectReference> ConstantPoolEntry::AsArrayOfChars() const
{
  if ( GetType() != e_ConstantPoolEntryTypeString )
  {
    throw TypeMismatchException( __FUNCTION__ " - Constant pool entry is not a string as expected." );
  }

  JavaString *pStringValue = dynamic_cast<JavaString *>( m_pValue.get() );

  std::shared_ptr<IThreadManager> pThreadManager = GlobalCatalog::GetInstance().Get( "ThreadManager" );
  boost::intrusive_ptr<ObjectReference> pArray = pThreadManager->GetCurrentThreadState()->CreateArray( e_JavaArrayTypes::Char, pStringValue->GetLengthInCodePoints() );


  for ( size_t i = 0; i < pStringValue->GetLengthInCodePoints(); ++ i )
  {
    pArray->GetContainedArray()->SetAt( JavaInteger::FromHostInt32( i ), JavaChar::FromCWChar( pStringValue->At( i ) ) );
  }

  return pArray;
}