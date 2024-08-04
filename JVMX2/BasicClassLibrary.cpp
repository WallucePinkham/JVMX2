#include "InvalidArgumentException.h"
#include "InvalidStateException.h"

#include "BasicClassLibrary.h"

BasicClassLibrary::~BasicClassLibrary()
{}

void BasicClassLibrary::AddClass( std::shared_ptr<JavaClass> pClass )
{
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );
  m_Classes[ *pClass->GetName() ] = pClass;
}

std::shared_ptr<JavaClass> BasicClassLibrary::FindClass( const JavaString &className ) const
{
  std::lock_guard<std::recursive_mutex> lock( m_Mutex );

  ClassContainer::const_iterator pos = m_Classes.find( className );
  if ( m_Classes.cend() != pos )
  {
    return (*pos).second;
  }

  return nullptr;
}

std::shared_ptr<ConstantPoolEntry> BasicClassLibrary::GetConstant( const JavaString &className, size_t index ) const
{
  std::shared_ptr<JavaClass> pClass = FindClass( className );
  if ( nullptr == pClass )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected class to be loaded already." );
  }

  return std::make_shared<ConstantPoolEntry>( pClass->GetConstant( index ) );
}

std::shared_ptr<MethodInfo> BasicClassLibrary::GetMethod( const JavaString &className, size_t index ) const
{
  std::shared_ptr<JavaClass> pFile = FindClass( className );

  return pFile->GetMethodByIndex( index );
}

std::shared_ptr<FieldInfo> BasicClassLibrary::GetField( const JavaString &className, size_t index ) const
{
  std::shared_ptr<JavaClass> pFile = FindClass( className );

  return pFile->GetFieldByIndex( index );
}

bool BasicClassLibrary::IsClassInitalised( const JavaString &className ) const
{
  std::shared_ptr<JavaClass> pClass = FindClass( className );
  if ( nullptr != pClass && pClass->IsInitialsed() )
  {
    return true;
  }

  return false;
}

std::vector<boost::intrusive_ptr<IJavaVariableType>> BasicClassLibrary::GetAllStaticObjectsAndArrays() const
{
  std::vector<boost::intrusive_ptr<IJavaVariableType>> statics;

  for ( auto clazz : m_Classes )
  {
    std::vector<boost::intrusive_ptr<IJavaVariableType>> classStatics = clazz.second->GetAllStaticObjectsAndArrays();
    statics.insert( statics.cend(), classStatics.cbegin(), classStatics.cend() );
  }

  return statics;
}
