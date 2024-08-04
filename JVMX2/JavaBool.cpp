
#include "JavaString.h"

#include "JavaBool.h"
#include "InvalidArgumentException.h"
#include "TypeParser.h"

static const bool c_DefaultBoolValue = false;

JavaBool::JavaBool( bool value )
  : m_Value( value )
{
}

JavaBool::JavaBool()
  : m_Value( c_DefaultBoolValue )
{
}

JavaBool::JavaBool( const JavaBool &other )
  : m_Value( other.m_Value )
{
}


e_JavaVariableTypes JavaBool::GetVariableType() const
{
  return e_JavaVariableTypes::Bool;
}

JavaBool JavaBool::FromBool( bool value )
{
  return JavaBool( value );
}

JavaBool JavaBool::FromUint16( int16_t value )
{
  return JavaBool( value != 0 );
}

bool JavaBool::ToBool() const
{
  return m_Value;
}

int16_t JavaBool::ToUint16() const
{
  return m_Value != 0 ? 1 : 0;
}

bool JavaBool::operator==(const JavaBool &other) const
{
  return m_Value == other.m_Value;
}

bool JavaBool::operator==(const IJavaVariableType &other) const
{
  if ( GetVariableType() != other.GetVariableType() )
  {
    return false;
  }

  return *this == *dynamic_cast<const JavaBool *>(&other);
}

JavaBool JavaBool::FromDefault()
{
  return JavaBool();
}

bool JavaBool::operator<(const IJavaVariableType &other) const
{
  if ( GetVariableType() == other.GetVariableType() )
  {
    return *this < *dynamic_cast<const JavaBool *>(&other);
  }

  return false;
}

bool JavaBool::operator<(const JavaBool &other) const
{
  return m_Value < other.m_Value;
}

JavaBool & JavaBool::operator=( const IJavaVariableType &other )
{

  boost::intrusive_ptr<JavaBool> pResult = nullptr;

  if ( !other.IsIntegerCompatible() )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected a type that was integer compatible." );
  }

  pResult = boost::dynamic_pointer_cast<JavaBool>(TypeParser::DownCastFromInteger( TypeParser::UpCastToInteger( &other ), e_JavaVariableTypes::Bool ));

  m_Value = pResult->m_Value;
  return *this;
  return *this;
}

bool JavaBool::IsReferenceType() const
{
  return false;
}

bool JavaBool::IsIntegerCompatible() const
{
  return true;
}

JavaString JavaBool::ToString() const
{
  return JavaString::FromCString( m_Value ? "{true}" : "{false}" );
}

bool JavaBool::IsNull() const
{
  return false;
}
