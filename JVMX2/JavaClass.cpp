
#include "InvalidArgumentException.h"
#include "InternalErrorException.h"
#include "FileInvalidException.h"
#include "TypeMismatchException.h"
#include "IndexOutOfBoundsException.h"
#include "InvalidStateException.h"
#include "NotImplementedException.h"
#include "HelperTypes.h"
#include "DefaultClassLoader.h"

#include "ObjectReference.h"

#include "AttributeConstantValue.h"

#include "JavaTypes.h"

#include "JavaClass.h"
#include "GlobalCatalog.h"
#include "IClassLibrary.h"

const ConstantPoolIndex c_DefaultIndex = 1;

extern const JavaString c_JavaLangClassName;

JavaClass::JavaClass( uint16_t minorVersion, uint16_t majorVersion, std::shared_ptr<ConstantPool> pConstantPool, uint16_t accessFlags, ConstantPoolIndex thisClassIndex, ConstantPoolIndex superClassIndex, InterfaceInfoList interfaces, FieldInfoList fields, MethodInfoList methods, CodeAttributeList attributes, boost::intrusive_ptr<ObjectReference> pClassLoader )
  : m_pClassName( nullptr )
  , m_pSuperClassName( nullptr )
  , m_pClassLoader( pClassLoader )
  , m_AccessFlags( accessFlags )
  , m_MinorVersion( minorVersion )
  , m_MajorVersion( majorVersion )
  , m_pConstantPool( pConstantPool )
  , m_ThisClassReferenceIndex( thisClassIndex )
  , m_SuperClassReferenceIndex( superClassIndex )
  , m_Interfaces( std::move( interfaces ) )
  , m_Fields( std::move( fields ) )
  , m_Methods( std::move( methods ) )
  , m_Attributes( std::move( attributes ) )
  , m_Initialised( false )
  , m_Initialising( false )
  , m_pMonitor( std::make_shared<Lockable>() )
{
  if ( nullptr == pConstantPool )
  {
    throw InvalidArgumentException(
      __FUNCTION__ " - NULL constant pool was passed on creation of JavaClassFile" );
  }

  SetupClassName();
  SetupSuperClassName();
  SetupMethods();
  //SetupSuperClass();
}

JavaClass::JavaClass( const JavaClass &other )
  : m_pClassName( other.m_pClassName )
  , m_pSuperClassName( other.m_pSuperClassName )
  , m_pClassLoader( other.m_pClassLoader )
  , m_pSuperClass( other.m_pSuperClass )
  , m_AccessFlags( other.m_AccessFlags )
  , m_MinorVersion( other.m_MinorVersion )
  , m_MajorVersion( other.m_MajorVersion )
  , m_ThisClassReferenceIndex( other.m_ThisClassReferenceIndex )
  , m_SuperClassReferenceIndex( other.m_SuperClassReferenceIndex )
  , m_Interfaces( other.m_Interfaces )
  , m_Fields( other.m_Fields )
  , m_Methods( other.m_Methods )
  , m_Attributes( other.m_Attributes )
  , m_Initialised( other.m_Initialised )
  , m_Initialising( other.m_Initialising )
  , m_pMonitor( std::make_shared<Lockable>() ) // NOT copying m_pMonitor
{
  m_pConstantPool = std::make_shared<ConstantPool>( *other.m_pConstantPool );
  SetupMethods();

  SetupSuperClass();
}

JavaClass::JavaClass( JavaClass &&other ) JVMX_NOEXCEPT
:
m_pSuperClass( other.m_pSuperClass )
, m_ThisClassReferenceIndex( c_DefaultIndex )
, m_SuperClassReferenceIndex( c_DefaultIndex )
{
  m_pConstantPool = nullptr;

  swap( *this, other );
  SetupMethods();

  SetupSuperClass();
}

JavaClass::~JavaClass()
{}

size_t JavaClass::CalculateInstanceSizeInBytes() const
{
  SetupSuperClass();

  size_t result = 0;

  for ( auto field : m_Fields )
  {
    if ( field->IsStatic() )
    {
      continue;
    }

    result += field->GetByteSize();
  }

  if ( nullptr != m_pSuperClass )
  {
    result += m_pSuperClass->CalculateInstanceSizeInBytes();
  }

  return result;
}

JavaClass &JavaClass::operator=( JavaClass other )
{
  swap( *this, other );

  return *this;
}

uint16_t JavaClass::GetMinorVersionNumber() const
{
  return m_MinorVersion;
}

uint16_t JavaClass::GetMajorVersionNumber() const
{
  return m_MajorVersion;
}

size_t JavaClass::GetConstantPoolCountAsDefinedByJava() const
{
  return m_pConstantPool->GetCount();
}

size_t JavaClass::GetNumberOfConstantsInConstantPool() const
{
  // Adding -1 for the 0 index which is invalid.
  return m_pConstantPool->GetCount() - 1;
}

ConstantPoolEntry JavaClass::GetConstant( size_t index ) const
{
  return *m_pConstantPool->GetConstant( index );
}

void JavaClass::swap( JavaClass &left, JavaClass &right ) JVMX_NOEXCEPT
{
  std::swap( left.m_MajorVersion, right.m_MajorVersion );
  std::swap( left.m_MinorVersion, right.m_MinorVersion );
  std::swap( left.m_pConstantPool, right.m_pConstantPool );
  std::swap( left.m_AccessFlags, right.m_AccessFlags );
  std::swap( left.m_ThisClassReferenceIndex, right.m_ThisClassReferenceIndex );
  std::swap( left.m_SuperClassReferenceIndex, right.m_SuperClassReferenceIndex );
  std::swap( left.m_Interfaces, right.m_Interfaces );
  std::swap( left.m_Fields, right.m_Fields );
  std::swap( left.m_Methods, right.m_Methods );
  std::swap( left.m_Attributes, right.m_Attributes );
  std::swap( left.m_pClassName, right.m_pClassName );
  std::swap( left.m_pSuperClassName, right.m_pSuperClassName );
  std::swap( left.m_pClassLoader, right.m_pClassLoader );
  std::swap( left.m_Initialised, right.m_Initialised );
  std::swap( left.m_Initialising, right.m_Initialising );
  std::swap( left.m_pMonitor, right.m_pMonitor );
}

bool JavaClass::IsPublic() const
{
  return 0 != ( m_AccessFlags & static_cast<uint16_t>( e_JavaClassAccessFlags::Public ) );
}

bool JavaClass::IsFinal() const
{
  return 0 != ( m_AccessFlags & static_cast<uint16_t>( e_JavaClassAccessFlags::Final ) );
}

bool JavaClass::IsSuper() const
{
  return 0 != ( m_AccessFlags & static_cast<uint16_t>( e_JavaClassAccessFlags::Super ) );
}

bool JavaClass::IsInterface() const
{
  return 0 != ( m_AccessFlags & static_cast<uint16_t>( e_JavaClassAccessFlags::Interface ) );
}

bool JavaClass::IsAbstract() const
{
  return 0 != ( m_AccessFlags & static_cast<uint16_t>( e_JavaClassAccessFlags::Abstract ) );
}

bool JavaClass::IsSynthetic() const
{
  return 0 != ( m_AccessFlags & static_cast<uint16_t>( e_JavaClassAccessFlags::Synthetic ) );
}

bool JavaClass::IsAnnotation() const
{
  return 0 != ( m_AccessFlags & static_cast<uint16_t>( e_JavaClassAccessFlags::Annotation ) );
}

bool JavaClass::IsEnum() const
{
  return 0 != ( m_AccessFlags & static_cast<uint16_t>( e_JavaClassAccessFlags::Enum ) );
}

boost::intrusive_ptr<JavaString> JavaClass::GetName() const
{
  return m_pClassName;
}

std::shared_ptr<JavaClass> JavaClass::GetSuperClass()
{
  SetupSuperClass();

  return m_pSuperClass;
}

boost::intrusive_ptr<JavaString> JavaClass::GetSuperClassName() const
{
  return m_pSuperClassName;
}

boost::intrusive_ptr<ObjectReference> JavaClass::GetClassLoader() const
{
  return m_pClassLoader;
}

size_t JavaClass::GetInterfacesCount() const
{
  return m_Interfaces.size();
}

const JavaString &JavaClass::GetInterfaceName( size_t interfaceIndex ) const
{
  try
  {
    return m_Interfaces.at( interfaceIndex ).GetName();
  }
  catch ( std::out_of_range & )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Could not get interface name. Index out of bounds." );
  }
}

size_t JavaClass::GetLocalFieldCount( e_PublicOnly publicOnly ) const
{
  if ( publicOnly != e_PublicOnly::Yes )
  {
    return m_Fields.size();
  }

  size_t fieldCount = 0;
  for ( auto field : m_Fields )
  {
    if ( field->IsPublic() )
    {
      ++ fieldCount;
    }
  }

  return fieldCount;
}

size_t JavaClass::GetNonStaticFieldCount() const
{
  size_t nonStaticFieldCount = 0;

  SetupSuperClass();

  if ( nullptr != m_pSuperClass )
  {
    nonStaticFieldCount = m_pSuperClass->GetNonStaticFieldCount();
  }

  for ( auto field : m_Fields )
  {
    if ( !field->IsStatic() )
    {
      ++ nonStaticFieldCount;
    }
  }

  return nonStaticFieldCount;
}

size_t JavaClass::GetLocalNonStaticFieldCount( e_PublicOnly publicOnly ) const
{
  size_t nonStaticFieldCount = 0;

  for ( auto field : m_Fields )
  {
    if ( field->IsStatic() )
    {
      continue; // do nothing;
    }

    if ( publicOnly == e_PublicOnly::Yes && !field->IsPublic() )
    {
      continue; // do nothing
    }

    ++ nonStaticFieldCount;
  }

  return nonStaticFieldCount;
}

void JavaClass::SetupSuperClass() const
{
  if ( nullptr == m_pSuperClass && !m_pSuperClassName->IsEmpty() )
  {
    std::shared_ptr<IClassLibrary> pLibrary = GlobalCatalog::GetInstance().Get( "ClassLibrary" );
    m_pSuperClass = pLibrary->FindClass( *m_pSuperClassName );

    if (nullptr == m_pSuperClass)
    {
      throw NotImplementedException("Not implemented yet");
    }
  }
}



size_t JavaClass::GetFieldCountIncludingSuperClasses() const
{
  SetupSuperClass();

  size_t superClassFieldCount = 0;
  if ( nullptr != m_pSuperClass )
  {
    superClassFieldCount = m_pSuperClass->GetFieldCountIncludingSuperClasses();
  }

  return superClassFieldCount + m_Fields.size();
}

std::shared_ptr<ConstantPoolNameAndTypeDescriptor> JavaClass::GetNameAndTypeDescriptorFromConstantPool( ConstantPoolIndex index ) const
{
  ConstantPoolEntry entry = GetConstant( index );

  if ( entry.GetType() != e_ConstantPoolEntryTypeNameAndTypeDescriptor )
  {
    throw TypeMismatchException( __FUNCTION__ " - Constant pool entry is not a name and type descriptor as expected." );
  }

  return entry.AsNameAndTypeDescriptorPointer();
}

std::shared_ptr<MethodInfo> JavaClass::GetMethodByIndex( size_t methodIndex ) const
{
  if ( methodIndex > m_Methods.size() || methodIndex < 0 )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Could not get Method name. Index out of bounds." );
  }

  size_t currentIndex = 0;

  for ( auto method : m_Methods )
  {
    if ( currentIndex == methodIndex )
    {
      return method.second;
    }

    ++ currentIndex;
  }

  throw InternalErrorException( __FUNCTION__ " - Method index not found. We should not have reached this." );
}

size_t JavaClass::GetMethodCount() const
{
  return m_Methods.size();
}

size_t JavaClass::GetAttributeCount() const
{
  return m_Attributes.size();
}

std::shared_ptr<JavaCodeAttribute> JavaClass::GetAttribute( size_t attributeIndex ) const
{
  try
  {
    return m_Attributes.at( attributeIndex );
  }
  catch ( std::out_of_range & )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Could not get Method. Index out of bounds." );
  }
}

uint16_t JavaClass::GetModifiers() const
{
  return m_AccessFlags;
}

std::shared_ptr<FieldInfo> JavaClass::GetFieldByIndex( size_t fieldIndex ) const
{
  try
  {
    return m_Fields.at( fieldIndex );
  }
  catch ( std::out_of_range & )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Could not get Filed. Index out of bounds." );
  }
}

JavaString JavaClass::GetPackageName() const
{
  return HelperTypes::GetPackageNameFromClassName( *m_pClassName );
}

std::shared_ptr<FieldInfo> JavaClass::GetFieldByName( const JavaString &fieldName ) const
{
  for ( auto field : m_Fields )
  {
    if ( fieldName == *( field->GetName() ) )
    {
      return field;
    }
  }

  return nullptr;
}

bool JavaClass::IsInitialsed() const
{
  std::lock_guard<std::recursive_mutex> pLock( m_InitialisationMutex );

  return m_Initialised || m_Initialising;
}

void JavaClass::SetupClassName()
{
  if ( 0 == m_ThisClassReferenceIndex )
  {
    throw FileInvalidException( __FUNCTION__ " - This Class Reference Index is 0, which is not valid." );
  }

  std::shared_ptr<ConstantPoolClassReference> entry = m_pConstantPool->GetConstant( m_ThisClassReferenceIndex )->AsClassReferencePointer();
  m_pClassName = m_pConstantPool->GetConstant( entry->ToConstantPoolIndex() )->AsString();
}

void JavaClass::SetupSuperClassName()
{
  if ( 0 == m_SuperClassReferenceIndex )
  {
    m_pSuperClassName = new JavaString( JavaString::EmptyString() );
    return;
  }

  std::shared_ptr<ConstantPoolClassReference> entry = m_pConstantPool->GetConstant( m_SuperClassReferenceIndex )->AsClassReferencePointer();
  m_pSuperClassName = m_pConstantPool->GetConstant( entry->ToConstantPoolIndex() )->AsString();
}

void JavaClass::SetInitialised()
{
  std::lock_guard<std::recursive_mutex> pLock( m_InitialisationMutex );

  m_Initialised = true;
  m_Initialising = false;
}

std::shared_ptr<MethodInfo> JavaClass::GetMethodByName( const JavaString &name ) const
{
  MethodInfoList::const_iterator pos = m_Methods.find( name );

  if ( pos != m_Methods.cend() )
  {
    return pos->second;
  }

  return nullptr;
}

std::shared_ptr<MethodInfo> JavaClass::GetMethodByNameAndType( const JavaString &name, const JavaString &type ) const
{
  auto range = m_Methods.equal_range( name );

  for ( MethodInfoList::const_iterator pos = range.first; pos != range.second; ++ pos )
  {
    if ( type == ( *pos->second->GetType() ) )
    {
      return pos->second;
    }
  }

  return nullptr;
}

size_t JavaClass::GetMethodIndexByNameAndType( const JavaString &name, const JavaString &type ) const
{
  size_t index = 0;
  for ( auto it = m_Methods.begin(); it != m_Methods.end(); ++ it )
  {
    if ( name == *( it->second )->GetName() && type == *( it->second )->GetType() )
    {
      return index;
    }

    ++ index;
  }

  throw InternalErrorException( __FUNCTION__ " - Method not found." );
}

void JavaClass::SetupMethods()
{
  for ( auto pPair : m_Methods )
  {
    pPair.second->SetClass( this );
  }
}

void JavaClass::SetInitialising()
{
  std::lock_guard<std::recursive_mutex> pLock( m_InitialisationMutex );

  JVMX_ASSERT( !m_Initialised && !m_Initialising );

  m_Initialising = true;
}

bool JavaClass::IsInitialsing() const
{
  std::lock_guard<std::recursive_mutex> pLock( m_InitialisationMutex );

  return m_Initialising;
}

std::shared_ptr< Lockable > JavaClass::MonitorEnter( const char *pFunctionName )
{
  m_pMonitor->Lock( pFunctionName );
  return m_pMonitor;
}

void JavaClass::MonitorExit( const char *pFunctionName )
{
  m_pMonitor->Unlock( pFunctionName );
}

std::vector<boost::intrusive_ptr<IJavaVariableType>> JavaClass::GetAllStaticObjectsAndArrays() const
{
  std::vector<boost::intrusive_ptr<IJavaVariableType>> statics;

  for ( auto field : m_Fields )
  {
    if ( field->IsStatic() )
    {
      const boost::intrusive_ptr<IJavaVariableType> value = field->GetStaticValue();
      if ( e_JavaVariableTypes::Object == value->GetVariableType() ||
           e_JavaVariableTypes::Array == value->GetVariableType() )
      {
        statics.push_back( value );
      }
    }
  }

  return statics;
}

std::recursive_mutex &JavaClass::GetInitialisationMutex()
{
  return m_InitialisationMutex;
}

std::shared_ptr<ConstantPool> JavaClass::GetConstantPool()
{
  return m_pConstantPool;
}