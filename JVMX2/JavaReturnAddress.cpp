#include "InvalidArgumentException.h"

#include "JavaString.h"

#include "JavaReturnAddress.h"

JavaReturnAddress::JavaReturnAddress( uintptr_t programCounter )
  : m_pAddress( programCounter )
{
}

JavaReturnAddress::JavaReturnAddress( const JavaReturnAddress &other )
  : m_pAddress( other.m_pAddress )
{}

JavaReturnAddress::~JavaReturnAddress() JVMX_NOEXCEPT
{}

bool JavaReturnAddress::operator==( const JavaReturnAddress &other ) const
{
  return m_pAddress == other.m_pAddress;
}

bool JavaReturnAddress::operator==( const IJavaVariableType &other ) const
{
  if ( GetVariableType() != other.GetVariableType() )
  {
    return false;
  }

  return *this == *dynamic_cast<const JavaReturnAddress *>(&other);
}

uintptr_t JavaReturnAddress::GetAddress() const JVMX_NOEXCEPT
{
  return m_pAddress;
}

e_JavaVariableTypes JavaReturnAddress::GetVariableType() const
{
  return e_JavaVariableTypes::ReturnAddress;
}

bool JavaReturnAddress::operator<( const IJavaVariableType &other ) const
{
  if ( GetVariableType() == other.GetVariableType() )
 {
    return *this < *dynamic_cast<const JavaReturnAddress *>(&other);
  }

  return false;
}

bool JavaReturnAddress::operator<( const JavaReturnAddress &other ) const
{
  // Deliberately comparing pointers.
  return m_pAddress < other.m_pAddress;
}

bool JavaReturnAddress::IsReferenceType() const
{
  return true;
}

bool JavaReturnAddress::IsIntegerCompatible() const
{
  return false;
}

JavaString JavaReturnAddress::ToString() const
{
  return JavaString::FromCString( JVMX_T( "{" ) ).Append( JavaString::FromCString( std::to_string( m_pAddress ).c_str() ) ).Append( JVMX_T( "}" ) );
}

bool JavaReturnAddress::IsNull() const
{
  return 0 == m_pAddress;
}