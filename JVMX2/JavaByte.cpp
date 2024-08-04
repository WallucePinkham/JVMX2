
#include "Endian.h"

#include "JavaString.h"

#include "InvalidArgumentException.h"
#include "TypeParser.h"

#include "JavaByte.h"


static const int8_t c_DefaultByteValue = 0;
static const int8_t c_IToABufferSize = 38;


JavaByte::JavaByte( int8_t value )
  : m_Value( value )
{}

JavaByte::JavaByte()
  : m_Value( c_DefaultByteValue )
{}

JavaByte::JavaByte( const JavaByte &other )
  : m_Value( other.m_Value )
{}

JavaByte::~JavaByte() JVMX_NOEXCEPT
{
}

JavaByte JavaByte::FromHostInt8( int8_t hostValue )
{
  return JavaByte( hostValue );
}

JavaByte JavaByte::FromNetworkInt8( const int8_t networkValue )
{
  return JavaByte( networkValue );
}

int8_t JavaByte::ToHostInt8() const
{
  return m_Value;
}

int8_t JavaByte::ToNetworkInt8() const
{
  return m_Value;
}

bool JavaByte::operator==(const JavaByte &other) const
{
  return m_Value == other.m_Value;
}

bool JavaByte::operator==(const IJavaVariableType &other) const
{
  if ( GetVariableType() != other.GetVariableType() )
  {
    return false;
  }

  return *this == *dynamic_cast<const JavaByte *>(&other);
}

e_JavaVariableTypes JavaByte::GetVariableType() const
{
  return e_JavaVariableTypes::Byte;
}

JavaByte JavaByte::FromDefault()
{
  return JavaByte();
}

bool JavaByte::operator<(const IJavaVariableType &other) const
{
  if ( GetVariableType() == other.GetVariableType() )
  {
    return *this < *dynamic_cast<const JavaByte *>(&other);
  }

  return false;
}

bool JavaByte::operator<(const JavaByte &other) const
{
  return m_Value < other.m_Value;
}

JavaByte &JavaByte::operator=( const IJavaVariableType &other )
{
  boost::intrusive_ptr<JavaByte> pResult = nullptr;

  if ( !other.IsIntegerCompatible() )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected a type that was integer compatible." );
  }

  pResult = boost::dynamic_pointer_cast<JavaByte>(TypeParser::DownCastFromInteger( TypeParser::UpCastToInteger( &other ), e_JavaVariableTypes::Byte ));

  m_Value = pResult->m_Value;
  return *this;
}

bool JavaByte::IsReferenceType() const
{
  return false;
}

bool JavaByte::IsIntegerCompatible() const
{
  return true;
}

JavaString JavaByte::ToString() const
{
  char dest[ c_IToABufferSize ] = { 0 };
  return JavaString::FromCString( _itoa( m_Value, dest, 10 ) );
}

bool JavaByte::IsNull() const
{
  return false;
}
