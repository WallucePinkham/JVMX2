
#include <codecvt>
#include <cstdint>

#include "NotImplementedException.h"

#include "HelperConversion.h"


std::wstring HelperConversion::ConvertUtf16StringToWideString( const char16_t *pFileName )
{
#if WCHAR_MAX == 0xFFFF
    return reinterpret_cast<const wchar_t *>(pFileName);
#elif WCHAR_MAX == 0xFFFFFFFF
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convertToUtf8;
    std::string utf8 = convertToUtf8.to_bytes( pFileName );

    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convertToUtf32;
    std::wstring result = convertToUtf32.from_bytes( utf8 );
#else
#error This still needs to be implemented for your platform.
#endif
}

std::string HelperConversion::ConvertUtf16StringToUtf8String( const char16_t *pFileName )
{
#if _MSC_VER == 1900
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convertToUtf8;
  return convertToUtf8.to_bytes( reinterpret_cast<const wchar_t *>(pFileName) );
#else
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convertToUtf8;
  return convertToUtf8.to_bytes( pFileName );
#endif
}

std::string HelperConversion::ConvertWideStringToUtf8String( const wchar_t * pFileName )
{
#if WCHAR_MAX == 0xFFFF  
  return ConvertUtf16StringToUtf8String( reinterpret_cast<const char16_t *>(pFileName) );
#elif WCHAR_MAX == 0xFFFFFFFF
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> utf32conv;
  return utf32conv.to_bytes( pFileName );
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
