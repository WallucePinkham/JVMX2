#include "Endian.h"

#include "InvalidArgumentException.h"
#include "TypeParser.h"

#include "JavaString.h"
#include "JavaLong.h"

static const uint64_t c_DefaultLongValue = 0;

JavaLong::JavaLong()
  : m_Value( c_DefaultLongValue )
{}

JavaLong::JavaLong( int64_t value )
  : m_Value( value )
{}

JavaLong::JavaLong( const JavaLong &other )
  : m_Value( other.m_Value )
{}

JavaLong::~JavaLong() JVMX_NOEXCEPT
{}

JavaLong JavaLong::FromHostInt64( int64_t hostValue )
{
  return JavaLong( hostValue );
}

JavaLong JavaLong::FromNetworkInt64( int64_t networkValue )
{
  return JavaLong( Endian::ntohll( networkValue ) );
}

int64_t JavaLong::ToHostInt64() const
{
  return m_Value;
}

bool JavaLong::operator==( const JavaLong &other ) const
{
  return m_Value == other.m_Value;
}

bool JavaLong::operator==( const IJavaVariableType &other ) const
{
  if ( GetVariableType() != other.GetVariableType() )
  {
    return false;
  }

  return *this == *dynamic_cast<const JavaLong *>(&other);
}

int64_t JavaLong::ToNetworkInt64() const
{
  return Endian::ntohll( m_Value );
}

e_JavaVariableTypes JavaLong::GetVariableType() const
{
  return e_JavaVariableTypes::Long;
}

JavaLong JavaLong::FromDefault()
{
  return JavaLong();
}

bool JavaLong::operator<( const IJavaVariableType &other ) const
{
  if ( GetVariableType() == other.GetVariableType() )
  {
    return *this < *dynamic_cast<const JavaLong *>(&other);
  }

  return false;
}

bool JavaLong::operator<( const JavaLong &other ) const
{
  return m_Value < other.m_Value;
}

JavaLong & JavaLong::operator=( const IJavaVariableType &other )
{
  if ( e_JavaVariableTypes::Long == other.GetVariableType() )
  {
    m_Value = dynamic_cast<const JavaLong *>( &other )->m_Value;
  }
  else if ( other.IsIntegerCompatible() )
  {
    m_Value = TypeParser::UpCastToInteger( &other )->ToHostInt32();
  }
  else
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected a type that was integer compatible." );
  }

  return *this;
}

bool JavaLong::IsReferenceType() const
{
  return false;
}

bool JavaLong::IsIntegerCompatible() const
{
  return false;
}

JavaString JavaLong::ToString() const
{
  char dest[ 34 ] = { 0 };
  return JavaString::FromCString( _i64toa( m_Value, dest, 10 ) );
}

bool JavaLong::IsNull() const
{
  return false;
}

JavaLong &JavaLong::operator=( const JavaLong &other )
{
  m_Value = other.m_Value;
  return *this;
}