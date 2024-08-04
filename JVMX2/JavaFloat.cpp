
#include <cfloat>
#include <limits>

#include "JavaString.h"

#include "UnsupportedPlatformException.h"
#include "JavaFloat.h"
#include "Endian.h"

static const float c_DefaultFloatValue = 0;

JavaFloat::JavaFloat() JVMX_NOEXCEPT
:
m_Value( c_DefaultFloatValue )
{
  static_assert( std::numeric_limits<double>::is_iec559, "Unsupported Platform" );
}

JavaFloat::JavaFloat( float value ) JVMX_NOEXCEPT
:
m_Value( value )
{
  static_assert( std::numeric_limits<double>::is_iec559, "Unsupported Platform" );
}

JavaFloat::JavaFloat( const JavaFloat &other )
  : m_Value( other.m_Value )
{}

JavaFloat &JavaFloat::operator=( const JavaFloat &other )
{
  m_Value = other.m_Value;
  return *this;
}

JavaFloat::~JavaFloat() JVMX_NOEXCEPT
{
}

JavaFloat JavaFloat::FromNetworkValue( float networkValue )
{
  uint32_t endianFloat = Endian::ntohl( *reinterpret_cast<uint32_t *>( &networkValue ) );

  return JavaFloat( *reinterpret_cast<float *>( &endianFloat ) );
}

float JavaFloat::ToNetworkValue() const
{
  uint32_t endianFloat = Endian::htonl( *reinterpret_cast<const uint32_t *>( &m_Value ) );

  return *reinterpret_cast<float *>( &endianFloat );
}

bool JavaFloat::operator==( const JavaFloat &other ) const
{
  float difference = m_Value - other.m_Value;

  if ( 0 == difference )
  {
    return true;
  }

  if ( difference > 0 && difference < FLT_EPSILON )
  {
    return true;
  }

  if ( difference < 0 && difference > FLT_EPSILON )
  {
    return true;
  }

  return false;
}

bool JavaFloat::operator==( const IJavaVariableType &other ) const
{
  if ( GetVariableType() != other.GetVariableType() )
  {
    return false;
  }

  return *this == *dynamic_cast<const JavaFloat *>( &other );
}

JavaFloat JavaFloat::FromHostFloat( float hostValue )
{
  return JavaFloat( hostValue );
}

float JavaFloat::ToHostFloat() const
{
  return m_Value;
}

e_JavaVariableTypes JavaFloat::GetVariableType() const
{
  return e_JavaVariableTypes::Float;
}

JavaFloat JavaFloat::FromHostDouble( double hostValue )
{
  return JavaFloat( static_cast<float>( hostValue ) );
}

JavaFloat JavaFloat::FromDefault()
{
  return JavaFloat();
}

bool JavaFloat::operator<( const IJavaVariableType &other ) const
{
  if ( GetVariableType() == other.GetVariableType() )
  {
    return *this < *dynamic_cast<const JavaFloat *>( &other );
  }

  return false;
}

bool JavaFloat::operator<( const JavaFloat &other ) const
{
  return m_Value < other.m_Value;
}

bool JavaFloat::operator>( const JavaFloat &other ) const
{
  return m_Value > other.m_Value;
}

JavaFloat &JavaFloat::operator=( const IJavaVariableType &other )
{
  JVMX_ASSERT( e_JavaVariableTypes::Float == other.GetVariableType() );
  m_Value = dynamic_cast<const JavaFloat *>( &other )->m_Value;

  return *this;
}

bool JavaFloat::IsReferenceType() const
{
  return false;
}

bool JavaFloat::IsIntegerCompatible() const
{
  return false;
}

JavaString JavaFloat::ToString() const
{
  return ConvertDoubleToString( m_Value );
}

bool JavaFloat::IsNan() const
{
  return _isnan( m_Value ) != 0;
}

bool JavaFloat::IsNull() const
{
  return false;
}

