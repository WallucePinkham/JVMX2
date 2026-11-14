
#include <codecvt>
#include <cstdint>
#include <locale>
#include <stdexcept>
#include <string>

#include "NotImplementedException.h"

#include "HelperConversion.h"


std::wstring HelperConversion::ConvertUtf16StringToWideString( const char16_t *pFileName )
{
#if WCHAR_MAX == 0xFFFF
  return std::wstring( reinterpret_cast<const wchar_t *>( pFileName ) );
#elif WCHAR_MAX > 0xFFFF
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convertToUtf8;
  std::string utf8 = convertToUtf8.to_bytes( pFileName );

  std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> convertToWide;
  return convertToWide.from_bytes( utf8 );
#else
#error This still needs to be implemented for your platform.
#endif
}

std::string HelperConversion::ConvertUtf16StringToUtf8String( const char16_t *pFileName )
{
#if WCHAR_MAX == 0xFFFF
//#if _MSC_VER == 1900
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convertToUtf8;
  return convertToUtf8.to_bytes( reinterpret_cast<const wchar_t *>(pFileName) );
#else
  static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convertToUtf8;
  return convertToUtf8.to_bytes( pFileName );
#endif
}

std::string HelperConversion::ConvertWideStringToUtf8String( const wchar_t * pFileName )
{
#if WCHAR_MAX == 0xFFFF  
  return ConvertUtf16StringToUtf8String( reinterpret_cast<const char16_t *>(pFileName) );
#elif WCHAR_MAX > 0xFFFF
  std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> convertToUtf8;
  return convertToUtf8.to_bytes( pFileName );
#else
#error This still needs to be implemented for your platform.
#endif
}

std::string HelperConversion::ConvertJVMXCharToUtf8( const JVMX_ANSI_CHAR_TYPE * pFileName )
{
  return std::string( pFileName );
}

std::string HelperConversion::ConvertJVMXCharToUtf8( const JVMX_WIDE_CHAR_TYPE *pFileName )
{
  return ConvertUtf16StringToUtf8String( pFileName );
}

std::u16string HelperConversion::ConvertUtf8ByteArrayToUtf16String( const uint8_t *pBuffer, size_t length )
{
#if _MSC_VER == 1900
  std::wstring converted;

  {
    std::wstring_convert< std::codecvt_utf8_utf16<wchar_t>, wchar_t > converter;
    converted = converter.from_bytes( reinterpret_cast<const char *>(pBuffer), reinterpret_cast<const char *>(pBuffer) + length );
  }

  const char16_t *pVal = reinterpret_cast<const char16_t *>(converted.c_str());

  std::u16string result = std::u16string( pVal, converted.length() );

  return result;
#else
  std::string utf8String( reinterpret_cast<const char *>(pBuffer), length );
  std::wstring_convert< std::codecvt_utf8_utf16<char16_t>, char16_t > converter;

  return converter.from_bytes( utf8String );
#endif 
}

std::u16string HelperConversion::ConvertModifiedUtf8ByteArrayToUtf16String( const uint8_t *pBuffer, size_t length )
{
  if ( nullptr == pBuffer && 0 != length )
  {
    throw std::invalid_argument( "Modified UTF-8 buffer is null." );
  }

  std::u16string result;
  size_t position = 0;
  while ( position < length )
  {
    const uint8_t first = pBuffer[position++];
    uint16_t codeUnit;

    if ( 0x01 <= first && first <= 0x7F )
    {
      codeUnit = first;
    }
    else if ( ( first & 0xE0 ) == 0xC0 )
    {
      if ( position >= length )
      {
        throw std::range_error( "Truncated modified UTF-8 sequence." );
      }

      const uint8_t second = pBuffer[position++];
      if ( ( second & 0xC0 ) != 0x80 )
      {
        throw std::range_error( "Invalid modified UTF-8 continuation byte." );
      }

      codeUnit = static_cast<uint16_t>( ( ( first & 0x1F ) << 6 ) | ( second & 0x3F ) );
      if ( codeUnit < 0x80 && !( first == 0xC0 && second == 0x80 ) )
      {
        throw std::range_error( "Overlong modified UTF-8 sequence." );
      }
    }
    else if ( ( first & 0xF0 ) == 0xE0 )
    {
      if ( position + 1 >= length )
      {
        throw std::range_error( "Truncated modified UTF-8 sequence." );
      }

      const uint8_t second = pBuffer[position++];
      const uint8_t third = pBuffer[position++];
      if ( ( second & 0xC0 ) != 0x80 || ( third & 0xC0 ) != 0x80 )
      {
        throw std::range_error( "Invalid modified UTF-8 continuation byte." );
      }

      codeUnit = static_cast<uint16_t>( ( ( first & 0x0F ) << 12 ) | ( ( second & 0x3F ) << 6 ) | ( third & 0x3F ) );
      if ( codeUnit < 0x800 )
      {
        throw std::range_error( "Overlong modified UTF-8 sequence." );
      }
    }
    else
    {
      throw std::range_error( "Invalid modified UTF-8 leading byte." );
    }

    result.push_back( static_cast<char16_t>( codeUnit ) );
  }

  return result;
}
