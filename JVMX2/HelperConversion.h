
#ifndef _HELPERCONVERSION__H_
#define _HELPERCONVERSION__H_

class HelperConversion
{
public:
  static std::wstring ConvertUtf16StringToWideString( const char16_t *pFileName );
  static std::string ConvertUtf16StringToUtf8String( const char16_t *pFileName );
  static std::string ConvertWideStringToUtf8String( const wchar_t *pFileName );

  static std::u16string ConvertUtf8ByteArrayToUtf16String( const uint8_t *pBuffer, size_t length );

  static std::string ConvertJVMXCharToUtf8( const JVMX_ANSI_CHAR_TYPE *pFileName );
  static std::string ConvertJVMXCharToUtf8( const JVMX_WIDE_CHAR_TYPE *pFileName );
};

#endif // _HELPERCONVERSION__H_
