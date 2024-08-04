#include "InvalidArgumentException.h"

#include "Endian.h"

#include "JavaString.h"
#include "JavaChar.h"
#include "JavaByte.h"
#include "JavaBool.h"
#include "JavaShort.h"

#include "JavaInteger.h"
#include "TypeParser.h"

static const int32_t c_DefaultIntegerValue = 0;
static const int32_t c_IToABufferSize = 38;

JavaInteger::JavaInteger()
  : m_Value( c_DefaultIntegerValue )
{}

JavaInteger::JavaInteger( int32_t value )
  : m_Value( value )
{}

JavaInteger::JavaInteger( const JavaInteger &other )
  : m_Value( other.m_Value )
{}

JavaInteger::~JavaInteger() JVMX_NOEXCEPT
{}

JavaInteger JavaInteger::FromHostInt32( int32_t hostValue )
{
  return JavaInteger( hostValue );
}

int32_t JavaInteger::ToHostInt32() const
{
  return m_Value;
}

bool JavaInteger::operator==( const JavaInteger &other ) const
{
  return m_Value == other.m_Value;
}

bool JavaInteger::operator==( const IJavaVariableType &other ) const
{
  if ( GetVariableType() != other.GetVariableType() )
  {
    return false;
  }

  return *this == *dynamic_cast<const JavaInteger *>(&other);
}

JavaInteger JavaInteger::FromNetworkInt32( const int32_t networkValue )
{
  return JavaInteger( (int32_t)Endian::ntohl( (uint32_t)networkValue ) );
}

int32_t JavaInteger::ToNetworkInt32() const
{
  return (int32_t)Endian::htonl( (uint32_t)m_Value );
}

e_JavaVariableTypes JavaInteger::GetVariableType() const
{
  return e_JavaVariableTypes::Integer;
}

JavaInteger JavaInteger::FromDefault()
{
  return JavaInteger();
}

bool JavaInteger::operator<( const IJavaVariableType &other ) const
{
  if ( GetVariableType() == other.GetVariableType() )
  {
    return *this < *dynamic_cast<const JavaInteger *>(&other);
  }

  return false;
}

bool JavaInteger::operator<( const JavaInteger &other ) const
{
  return m_Value < other.m_Value;
}

bool JavaInteger::IsReferenceType() const
{
  return false;
}

bool JavaInteger::IsIntegerCompatible() const
{
  return true;
}

boost::intrusive_ptr<JavaChar> JavaInteger::ToChar() const
{
  return new JavaChar( JavaChar::FromCChar( static_cast<char>(ToHostInt32()) ) );
}

boost::intrusive_ptr<JavaByte> JavaInteger::ToByte() const
{
  return new JavaByte( JavaByte::FromHostInt8( static_cast<int8_t>(ToHostInt32()) ) );
}

boost::intrusive_ptr<JavaBool> JavaInteger::ToBool() const
{
  return new JavaBool( JavaBool::FromBool( 0 != ToHostInt32() ) );
}

boost::intrusive_ptr<JavaShort> JavaInteger::ToShort() const
{
  return new JavaShort( JavaShort::FromHostInt16( static_cast<int16_t>(ToHostInt32()) ) );
}

JavaInteger JavaInteger::FromChar( const JavaChar &value )
{
  return JavaInteger( value.ToUInt16() );
}

JavaInteger JavaInteger::FromBool( const JavaBool &value  )
{
  return JavaInteger( value.ToUint16() );
}

JavaInteger JavaInteger::FromByte( const JavaByte &value )
{
  return JavaInteger( value.ToHostInt8() );
}

JavaInteger JavaInteger::FromShort( const JavaShort &value )
{
  return JavaInteger( value.ToHostInt16() );
}

JavaString JavaInteger::ToString() const
{
  char dest[ c_IToABufferSize ] = { 0 };
  return JavaString::FromCString( _itoa( m_Value, dest, 10 ) );
}

bool JavaInteger::IsNull() const
{
  return false;
}

JavaInteger & JavaInteger::operator=( const JavaInteger &other )
{
  m_Value = other.m_Value;
  return *this;
}

JavaInteger &JavaInteger::operator=( const IJavaVariableType &other )
{
  if ( !other.IsIntegerCompatible() )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected a type that was integer compatible." );
  }

  m_Value = TypeParser::UpCastToInteger( &other )->ToHostInt32();

  return *this;
}