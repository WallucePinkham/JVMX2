#include <cstring>
#include <algorithm>
#include <functional>
#include <codecvt>

#include "IndexOutOfBoundsException.h"
#include "InvalidArgumentException.h"

#include "HelperConversion.h"

#include "DataBuffer.h"
#include "OsFunctions.h"

#include "JavaArray.h"
#include "JavaChar.h"

#include "JavaString.h"


static const size_t c_JavaStringWideCharSize = sizeof( wchar_t );
static const size_t c_JavaStringCharSize = sizeof( char );
static const size_t c_JavaStringNullChar = '\0';

typedef char JavaStringCharType;

// JavaString::JavaString( size_t length, const uint8_t *pBuffer )
//   : m_Data(  )
// {
//
// }

JavaString::JavaString( const JavaString &other )
  : m_Data( other.m_Data )
{}

JavaString::JavaString( JavaString &&other ) JVMX_NOEXCEPT
:
m_Data( std::move( other.m_Data ) )
{
}

JavaString::JavaString( std::u16string &&other ) JVMX_NOEXCEPT
:
m_Data( std::move( other ) )
{
}

JavaString::JavaString( const std::u16string &other ) JVMX_NOEXCEPT
:
m_Data( other )
{
}

JavaString::~JavaString() JVMX_NOEXCEPT
{}

JavaString JavaString::FromCString( const JVMX_WIDE_CHAR_TYPE *pString )
{
  return JavaString( pString );
}

JavaString JavaString::FromCString( const JVMX_ANSI_CHAR_TYPE *pString )
{
  std::u16string str = HelperConversion::ConvertUtf8ByteArrayToUtf16String( reinterpret_cast<const uint8_t *>( pString ), strlen( pString ) );
  return JavaString( str );
}

// const uint8_t *JavaString::ToByteArray() const
// {
//   return m_Data.c_str();
// }

JavaString &JavaString::operator=( const JavaString &other )
{
  if ( this != &other )
  {
    m_Data = other.m_Data;
  }

  return *this;
}

size_t JavaString::GetLengthInBytes() const
{
  return m_Data.get().length() * sizeof(char16_t);
}

size_t JavaString::GetLengthInCodePoints() const
{
  return m_Data.get().length();
}

JavaString JavaString::AppendHex( int32_t intVal ) const
{
  static char buffer[ 65 ];
  return this->Append( JavaString::FromCString( _itoa( intVal, buffer, 16 ) ) );
}

void JavaString::swap( JavaString &left, JavaString &right ) JVMX_NOEXCEPT
{
  std::swap( left.m_Data, right.m_Data );
}

bool JavaString::operator==( const JavaString &other ) const
{
  return m_Data == other.m_Data;
}

bool JavaString::operator==( const IJavaVariableType &other ) const
{
  if ( GetVariableType() != other.GetVariableType() )
  {
    return false;
  }

  return *this == *dynamic_cast<const JavaString *>( &other );
}

const JavaString JavaString::EmptyString()
{
  static const JavaString emptyString = JavaString::FromCString( JVMX_T( "" ) );
  return emptyString;
}

//JavaString JavaString::FromUtf16ByteArray( size_t length, const uint8_t *pBuffer )
//{
// return JavaString( std::u16string( pBuffer, length ) );
//}

JavaString JavaString::FromUtf8ByteArray( size_t length, const uint8_t *pBuffer )
{
  return JavaString( HelperConversion::ConvertUtf8ByteArrayToUtf16String( pBuffer, length ) );
}


DataBuffer JavaString::ToDataBuffer() const
{
  return DataBuffer::FromByteArray( m_Data.get().length(), reinterpret_cast<const uint8_t *>( m_Data.get().c_str() ) );
}

bool JavaString::IsEmpty() const JVMX_NOEXCEPT
{
  return m_Data.get().empty();
}

bool JavaString::operator<( const JavaString &other ) const
{
  return m_Data < other.m_Data;
}

bool JavaString::operator<( const IJavaVariableType &other ) const
{
  if ( GetVariableType() == other.GetVariableType() )
  {
    return *this < *dynamic_cast<const JavaString *>( &other );
  }

  return false;
}

JavaString &JavaString::operator=( const IJavaVariableType &other )
{
  JVMX_ASSERT( e_JavaVariableTypes::String == other.GetVariableType() );
  m_Data =  dynamic_cast<const JavaString *>( &other )->m_Data;

  return *this;
}

const char16_t *JavaString::ToCharacterArray() const
{
  return m_Data.get().c_str();
}

// JavaString JavaString::Append( const JVMX_ANSI_CHAR_TYPE *pString ) const
// {
//   return JavaString( m_Data + ConvertUtf8ByteArrayToUtf16String(reinterpret_cast<const uint8_t *>(pString), strlen(pString)) );
// }
//
// JavaString JavaString::Append( const JVMX_ANSI_CHAR_TYPE value ) const
// {
//   return JavaString( m_Data + ConvertUtf8ByteArrayToUtf16String( reinterpret_cast<const uint8_t *>(&value), 1) );
//   //return JavaString( m_Data + value );
// }

JavaString JavaString::Append( const JVMX_CHAR_TYPE *pString ) const
{
  return JavaString( m_Data.get() + pString );
}

JavaString JavaString::Append( const JVMX_CHAR_TYPE value ) const
{
  return JavaString( m_Data.get() + value );
}

JavaString JavaString::Append( const JavaString &pString ) const
{
  //std::u16string temp = m_Data;
  // temp.append( pString.m_Data );
  //temp.append( reinterpret_cast<const JVMX_ANSI_CHAR_TYPE *>(pString.ToByteArray()) );
  return JavaString( m_Data.get() + pString.m_Data.get() );
}



JavaString JavaString::Append( int32_t intVal ) const
{
  char buffer[ 65 ];
  return this->Append( JavaString::FromCString( _itoa( intVal, buffer, 10 ) ) );
}

/*JavaString JavaString::FromDataBuffer( DataBuffer dataBuffer )
{
return JavaString( dataBuffer.GetByteLength(), dataBuffer.ToByteArray() );
}*/

std::u16string JavaString::ToUtf16String() const
{
  return m_Data;
}

std::string JavaString::ToUtf8String() const
{
  return HelperConversion::ConvertUtf16StringToUtf8String( m_Data.get().c_str());
}   

char16_t JavaString::At( size_t index ) const
{
  if ( index > m_Data.get().length() )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Invalid index passed." );
  }

  return m_Data.get().at( index );
}

e_JavaVariableTypes JavaString::GetVariableType() const
{
  return e_JavaVariableTypes::String;
}

size_t JavaString::FindLast( JVMX_CHAR_TYPE charToFind ) const
{
  return m_Data.get().rfind( charToFind );
}

size_t JavaString::GetLastStringPosition() const
{
  return m_Data.get().npos;
}

bool JavaString::EndsWith( const JVMX_WIDE_CHAR_TYPE *pStringToMatch ) const
{
  std::u16string temp( pStringToMatch );
  auto pTempPos = temp.crbegin();

  for ( auto pos = m_Data.get().crbegin(); pos != m_Data.get().crend(); ++ pos )
  {
    if ( *pos != *pTempPos )
    {
      return false;
    }

    ++ pTempPos;
    if ( pTempPos == temp.crend() )
    {
      return true;
    }
  }

  if ( pTempPos == temp.crend() )
  {
    return true;
  }

  return false;
}

JavaString JavaString::SubString( size_t offset ) const
{
  return JavaString::FromCString( m_Data.get().substr( offset ).c_str() );
}

JavaString JavaString::SubString( size_t offset, size_t numberOfCharacters ) const
{
  return JavaString::FromCString( m_Data.get().substr( offset, numberOfCharacters ).c_str() );
}

bool JavaString::operator!=( const JavaString &other ) const
{
  return !( *this == other );
}

size_t JavaString::FindNext( size_t offset, JVMX_CHAR_TYPE charToFind ) const
{
  auto res = m_Data.get().find(charToFind, offset);
  return res;
}

JavaString JavaString::ReplaceAll( char16_t param1, char16_t param2 ) const
{
  std::u16string result = m_Data;
  std::replace( result.begin(), result.end(), param1, param2 );
  return JavaString( std::u16string( result ) );
}

bool JavaString::IsReferenceType() const
{
  return false;
}

bool JavaString::IsIntegerCompatible() const
{
  return false;
}

JavaString JavaString::ToString() const
{
  return *this;
}

bool JavaString::Contains( const char16_t *pString ) const
{
  return m_Data.get().find( pString ) != GetLastStringPosition();
}

bool JavaString::Equals( const char16_t *pString ) const
{
  return 0 == m_Data.get().compare( pString );
}

bool JavaString::IsNull() const
{
  return false;
}

JavaString JavaString::FromArray( const JavaArray &array )
{
  if ( e_JavaArrayTypes::Char != array.GetContainedType() )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected array to contain chars." );
  }

  std::u16string result;
  for ( size_t i = 0; i < array.GetNumberOfElements(); ++ i )
  {
    const JavaChar *pChar = dynamic_cast<const JavaChar *>( array.At( i ) );
    char16_t ch = pChar->ToChar16();

    // We assume that the null character ends the string.
    if ( u'\u0000' == ch )
    {
      break;
    }

    result += ch;
  }

  return JavaString( std::u16string( result ) );
}

size_t JavaString::Hash() const
{
  std::hash<std::u16string> hash_fn;
  return hash_fn( m_Data );
}

size_t JavaString::NotFound()
{
  return std::u16string::npos;
}

JavaString JavaString::FromChar( JVMX_ANSI_CHAR_TYPE c )
{
  return JavaString( HelperConversion::ConvertUtf8ByteArrayToUtf16String( reinterpret_cast<const uint8_t *>( &c ), c_JavaStringCharSize ) );
}

JavaString JavaString::FromWChar( JVMX_WIDE_CHAR_TYPE wc )
{
  return JavaString( std::u16string( &wc, 1 ) );
}
