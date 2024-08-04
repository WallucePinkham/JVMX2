
#include <limits>
#include <cfloat>
#include <memory>

#include "InternalErrorException.h"

#include "JavaString.h"

#include "Endian.h"
#include "JavaDouble.h"

static const double c_DefaultDoubleValue = 0;
static const int c_SignificantDigits = 12;


//static bool IsIeee754() { return std::numeric_limits<double>::is_iec559; }

JavaDouble::JavaDouble() JVMX_NOEXCEPT
:
m_Value( c_DefaultDoubleValue )
{
  static_assert( std::numeric_limits<double>::is_iec559, "Unsupported Platform" );
}


JavaDouble::JavaDouble( double value ) JVMX_NOEXCEPT
:
m_Value( value )
{
  static_assert( std::numeric_limits<double>::is_iec559, "Unsupported Platform" );
}

JavaDouble::JavaDouble( const JavaDouble &other )
  : m_Value( other.m_Value )
{}


JavaDouble::~JavaDouble() JVMX_NOEXCEPT
{
}

JavaDouble JavaDouble::FromNetworkValue( double hostValue )
{
  uint64_t endianFloat = Endian::ntohll( *reinterpret_cast<uint64_t *>( &hostValue ) );

  return JavaDouble( *reinterpret_cast<double *>( &endianFloat ) );
}

JavaDouble JavaDouble::FromHostInt64( int64_t intValue )
{
  return FromHostDouble( static_cast<double>( intValue ) );
}

JavaDouble JavaDouble::FromHostDouble( double hostValue )
{
  return JavaDouble( hostValue );
}

double JavaDouble::ToNetworkValue() const
{
  uint64_t endianFloat = Endian::htonll( *reinterpret_cast<const uint64_t *>( &m_Value ) );

  return *reinterpret_cast<double *>( &endianFloat );
}

double JavaDouble::ToHostDouble() const
{
  return m_Value;
}

bool JavaDouble::operator==( const JavaDouble &other ) const
{
  double difference = m_Value - other.m_Value;

  if ( 0 == difference )
  {
    return true;
  }

  if ( difference > 0 && difference < DBL_EPSILON )
  {
    return true;
  }

  if ( difference < 0 && difference > DBL_EPSILON )
  {
    return true;
  }

  return false;
}

bool JavaDouble::operator==( const IJavaVariableType &other ) const
{
  if ( GetVariableType() != other.GetVariableType() )
  {
    return false;
  }

  return *this == *dynamic_cast<const JavaDouble *>( &other );
}

e_JavaVariableTypes JavaDouble::GetVariableType() const
{
  return e_JavaVariableTypes::Double;
}

JavaDouble JavaDouble::FromDefault()
{
  return JavaDouble();
}

bool JavaDouble::operator<( const IJavaVariableType &other ) const
{
  if ( GetVariableType() == other.GetVariableType() )
  {
    return *this < *dynamic_cast<const JavaDouble *>( &other );
  }

  return false;
}

bool JavaDouble::operator<( const JavaDouble &other ) const
{
  return m_Value < other.m_Value;
}

bool JavaDouble::operator>( const JavaDouble &other ) const
{
  return m_Value > other.m_Value;
}

JavaDouble &JavaDouble::operator=( const IJavaVariableType &other )
{
  JVMX_ASSERT( e_JavaVariableTypes::Double == other.GetVariableType() );
  m_Value = dynamic_cast<const JavaDouble *>( &other )->m_Value;

  return *this;
}

bool JavaDouble::IsReferenceType() const
{
  return false;
}

bool JavaDouble::IsIntegerCompatible() const
{
  return false;
}

JavaString JavaDouble::ToString() const
{
  return ConvertDoubleToString( m_Value );
}

bool JavaDouble::IsNull() const
{
  return false;
}

JavaDouble &JavaDouble::operator=( const JavaDouble &other )
{
  m_Value = other.m_Value;
  return *this;
}

bool JavaDouble::IsNan() const
{
  return _isnan( m_Value ) != 0;
}

