
#include "Endian.h"

#include "JavaString.h"
#include "TypeParser.h"

#include "InvalidArgumentException.h"

#include "JavaShort.h"



static const int16_t c_DefaultShortValue = 0;

JavaShort::JavaShort( int16_t value )
  : m_Value( value )
{
}

JavaShort::JavaShort()
  : m_Value( c_DefaultShortValue )
{
}

JavaShort::JavaShort( const JavaShort &other )
  : m_Value( other.m_Value )
{}

JavaShort::~JavaShort() JVMX_NOEXCEPT
{
}

JavaShort JavaShort::FromHostInt16( int16_t hostValue )
{
  return JavaShort( hostValue );
}

JavaShort JavaShort::FromNetworkInt16( const int16_t networkValue )
{
  return JavaShort( Endian::htons( networkValue ) );
}

int16_t JavaShort::ToHostInt16() const
{
  return m_Value;
}

int16_t JavaShort::ToNetworkInt16() const
{
  return Endian::ntohs( m_Value );
}

bool JavaShort::operator==(const JavaShort &other) const
{
  return m_Value == other.m_Value;
}

bool JavaShort::operator==(const IJavaVariableType &other) const
{
  if ( GetVariableType() != other.GetVariableType() )
  {
    return false;
  }

  return *this == *dynamic_cast<const JavaShort *>(&other);
}

e_JavaVariableTypes JavaShort::GetVariableType() const
{
  return e_JavaVariableTypes::Short;
}

JavaShort JavaShort::FromDefault()
{
  return JavaShort();
}

bool JavaShort::operator<(const IJavaVariableType &other) const
{
  if ( GetVariableType() == other.GetVariableType() )
  {
    return *this < *dynamic_cast<const JavaShort *>(&other);
  }

  return false;
}

bool JavaShort::operator<(const JavaShort &other) const
{
  return m_Value < other.m_Value;
}

JavaShort &JavaShort::operator=( const IJavaVariableType &other )
{
  boost::intrusive_ptr<JavaShort> pResult = nullptr;

  if ( !other.IsIntegerCompatible() )
  {
    throw InvalidArgumentException(__FUNCTION__ " - Expected a type that was integer compatible." );
  }

  pResult = boost::dynamic_pointer_cast<JavaShort>( TypeParser::DownCastFromInteger( TypeParser::UpCastToInteger( &other ), e_JavaVariableTypes::Short ) );

  m_Value = pResult->m_Value;
  return *this;
}

bool JavaShort::IsReferenceType() const
{
  return false;
}

bool JavaShort::IsIntegerCompatible() const
{
  return true;
}

JavaString JavaShort::ToString() const
{
  char dest[ 38 ] = { 0 };
  return JavaString::FromCString( _itoa( m_Value, dest, 10 ) );
}

bool JavaShort::IsNull() const
{
  return false;
}
