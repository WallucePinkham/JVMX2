
#include "JavaString.h"

#include "JavaClass.h"
#include "JavaClassReference.h"

JavaClassReference::JavaClassReference( std::shared_ptr<JavaClass> pClassFile )
  : m_pClassFile( pClassFile )
{
  DebugAssert();
}

JavaClassReference::JavaClassReference( const JavaClassReference &other )
  : m_pClassFile( other.m_pClassFile )
{
  DebugAssert();
}

e_JavaVariableTypes JavaClassReference::GetVariableType() const
{
  DebugAssert();
  return e_JavaVariableTypes::ClassReference;
}

bool JavaClassReference::operator==( const JavaClassReference &other ) const
{
  DebugAssert();
  // Deliberately comparing pointers here.
  return m_pClassFile.get() == other.m_pClassFile.get();
}

bool JavaClassReference::operator==( const IJavaVariableType &other ) const
{
  DebugAssert();

  if ( GetVariableType() != other.GetVariableType() )
  {
    return false;
  }

  return *this == *dynamic_cast<const JavaClassReference *>( &other );
}

std::shared_ptr<JavaClass> JavaClassReference::GetClassFile() const
{
  DebugAssert();
  return m_pClassFile;
}

bool JavaClassReference::operator<( const IJavaVariableType &other ) const
{
  DebugAssert();
  if ( GetVariableType() == other.GetVariableType() )
  {
    return *this < *dynamic_cast<const JavaClassReference *>( &other );
  }

  return false;
}

bool JavaClassReference::operator<( const JavaClassReference &other ) const
{
  DebugAssert();
  // Deliberately comparing pointers here.
  return m_pClassFile.get() < other.m_pClassFile.get();
}

bool JavaClassReference::IsReferenceType() const
{
  DebugAssert();
  return true;
}

bool JavaClassReference::IsIntegerCompatible() const
{
  DebugAssert();
  return false;
}

JavaString JavaClassReference::ToString() const
{
  DebugAssert();

  if ( nullptr == m_pClassFile )
  {
    return JavaString::FromCString( JVMX_T( "{null}" ) );
  }

  return JavaString::FromCString( JVMX_T( "{" ) ).Append( *( m_pClassFile->GetName() ) ).Append( JVMX_T( "}" ) );
}

void JavaClassReference::DebugAssert() const
{
  JVMX_ASSERT( nullptr != m_pClassFile );
}

bool JavaClassReference::IsNull() const
{
  return nullptr == m_pClassFile;
}
