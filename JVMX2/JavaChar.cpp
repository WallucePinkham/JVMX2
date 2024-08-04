
#include <string>

#include "JavaString.h"
#include "InvalidArgumentException.h"

#include "OsFunctions.h"
#include "Endian.h"
#include "TypeParser.h"

#include "JavaChar.h"

static const int16_t c_DefaultCharValue = 0;

JavaChar::~JavaChar() JVMX_NOEXCEPT
{
}

// const uint8_t *JavaChar::ToByteArray() const
// {
//   return reinterpret_cast<const uint8_t*>(&m_Value);
// }

JavaChar &JavaChar::operator=(JavaChar other)
{
  swap( *this, other );

  return *this;
}

JavaChar &JavaChar::operator=( const IJavaVariableType &other )
{
  boost::intrusive_ptr<JavaChar> pResult = nullptr;

  if ( !other.IsIntegerCompatible() )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected a type that was integer compatible." );
  }

  if ( e_JavaVariableTypes::Char != other.GetVariableType() )
  {
    pResult = boost::dynamic_pointer_cast<JavaChar>(TypeParser::DownCastFromInteger( TypeParser::UpCastToInteger( &other ), e_JavaVariableTypes::Char ));
  }
  else
  {
    pResult = new JavaChar( *static_cast<const JavaChar *>(&other) );
  }

  m_Value = pResult->m_Value;
  return *this;
}

//
// size_t JavaChar::GetByteLength() const
// {
//   return sizeof( m_Value );
// }

void JavaChar::swap( JavaChar &left, JavaChar &right ) JVMX_NOEXCEPT
{
  std::swap( left.m_Value, right.m_Value );
}

bool JavaChar::operator==(const JavaChar &other) const
{
  return m_Value == other.m_Value;
}

bool JavaChar::operator==(const IJavaVariableType &other) const
{
  if ( GetVariableType() != other.GetVariableType() )
  {
    return false;
  }

  return *this == *dynamic_cast<const JavaChar *>(&other);
}

const JavaChar JavaChar::DefaultChar()
{
  return JavaChar();
}


JavaChar JavaChar::FromCChar( const char value )
{
  return JavaChar( static_cast<uint16_t>(value) );
}

JavaChar JavaChar::FromCWChar( const wchar_t value )
{
  static_assert(sizeof( wchar_t ) == sizeof( uint16_t ), "This might be interesting on Linux or any OS that uses UTF32. We'll need to test.");
  return JavaChar( static_cast<uint16_t>(value) );
}

JavaChar JavaChar::FromChar16( const char16_t value )
{
  return JavaChar( value );
}

JavaChar::JavaChar( char16_t value )
  : m_Value( value )
{
}

JavaChar::JavaChar( const JavaChar &other )
  : m_Value( other.m_Value )
{}

JavaChar::JavaChar( JavaChar &&other ) JVMX_NOEXCEPT
{
  swap( *this, other );
}

JavaChar::JavaChar()
: m_Value( c_DefaultCharValue )
{
}

JavaChar JavaChar::FromUInt16( uint16_t value )
{
  return JavaChar( value );
}

/*char JavaChar::ToCChar() const
{
  return static_cast<char>(m_Value);
}*/

char16_t JavaChar::ToChar16() const
{
  return static_cast<char16_t>(m_Value);
}

uint16_t JavaChar::ToUInt16() const
{
  return m_Value;
}

e_JavaVariableTypes JavaChar::GetVariableType() const
{
  return e_JavaVariableTypes::Char;
}

bool JavaChar::operator<(const IJavaVariableType &other) const
{
  if ( GetVariableType() == other.GetVariableType() )
  {
    return *this < *dynamic_cast<const JavaChar *>(&other);
  }

  return false;
}

bool JavaChar::operator<(const JavaChar &other) const
{
  return m_Value < other.m_Value;
}

JavaChar JavaChar::FromDefault()
{
  return JavaChar( '\0' );
}

bool JavaChar::IsReferenceType() const
{
  return false;
}

bool JavaChar::IsIntegerCompatible() const
{
  return true;
}

JavaString JavaChar::ToString() const
{
  char value = 0;

  if ( m_Value > 32 && m_Value < 126 )
  {
    value = static_cast<char>(m_Value);
  }
  else
  {
    value = '.';
  }

  return JavaString::FromChar( value );
}

bool JavaChar::IsNull() const
{
  return false;
}
