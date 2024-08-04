
#pragma once

#ifndef __JAVASTRING_H__
#define __JAVASTRING_H__

#include <string>
#include <boost/flyweight.hpp>

#include "GlobalConstants.h"

#include "IJavaVariableType.h"
#include "IConstantPoolEntryValue.h"
#include "DataBuffer.h"

class JavaArray;

class JavaString : public IJavaVariableType, public IConstantPoolEntryValue
{
public:
  static JavaString FromCString( const JVMX_ANSI_CHAR_TYPE *pString );
  static JavaString FromCString( const JVMX_WIDE_CHAR_TYPE *pString );
  static JavaString FromUtf8ByteArray( size_t length, const uint8_t *pBuffer );

  //static JavaString FromUtf16ByteArray( size_t length, const uint8_t *pBuffer );
  static JavaString FromArray( const JavaArray &array );
  static JavaString FromChar( JVMX_ANSI_CHAR_TYPE c );
  static JavaString FromWChar( JVMX_WIDE_CHAR_TYPE wc );

protected:
  //explicit JavaString( size_t length, const uint8_t *pBuffer );
  explicit JavaString( std::u16string &&other ) JVMX_NOEXCEPT;
  explicit JavaString( const std::u16string &other ) JVMX_NOEXCEPT;

//  static JavaString ConvertWideCharacterByteArrayToUtf16String( const wchar_t *pBuffer );
  

  //static JavaString FromDataBuffer( DataBuffer dataBuffer );

public:
  JavaString( const JavaString &other );
  explicit JavaString( JavaString &&other ) JVMX_NOEXCEPT;

  virtual ~JavaString() JVMX_NOEXCEPT;

  virtual e_JavaVariableTypes GetVariableType() const JVMX_OVERRIDE;
  virtual bool IsReferenceType() const JVMX_OVERRIDE;
  virtual bool IsIntegerCompatible() const JVMX_OVERRIDE;
  virtual bool IsNull() const JVMX_OVERRIDE;
  virtual JavaString ToString() const JVMX_OVERRIDE;

  JavaString &operator=( const JavaString &other );

  virtual bool operator==( const IJavaVariableType &other ) const JVMX_OVERRIDE;
  virtual bool operator==( const JavaString &other ) const;
  virtual bool operator!=( const JavaString &other ) const;

  virtual bool operator<( const JavaString &other ) const;
  virtual bool operator<( const IJavaVariableType &other ) const JVMX_OVERRIDE;

  virtual JavaString &operator=( const IJavaVariableType &other ) JVMX_OVERRIDE;

  const char16_t *ToCharacterArray() const;
  //const uint8_t *ToByteArray() const;
  size_t GetLengthInBytes() const;
  size_t GetLengthInCodePoints() const;

  //JavaString Append( const JVMX_ANSI_CHAR_TYPE *pString ) const;
  //JavaString Append( const JVMX_ANSI_CHAR_TYPE value ) const;

  JavaString AppendHex( int32_t intVal ) const;
  JavaString Append( int32_t intVal ) const;
  JavaString Append( const JVMX_CHAR_TYPE *pString ) const;
  JavaString Append( const JVMX_CHAR_TYPE value ) const;


  JavaString Append( const JavaString &pString ) const;

  DataBuffer ToDataBuffer() const;

  bool IsEmpty() const JVMX_NOEXCEPT;

  std::u16string ToUtf16String() const;
  std::string ToUtf8String() const;

  char16_t At( size_t index ) const;

  size_t FindNext( size_t offset, JVMX_CHAR_TYPE charToFind ) const;
  size_t FindLast( JVMX_CHAR_TYPE charToFind ) const;
  size_t GetLastStringPosition() const;

  bool EndsWith( const JVMX_WIDE_CHAR_TYPE *pStringToMatch ) const;

  JavaString SubString( size_t offset ) const;
  JavaString SubString( size_t offset, size_t numberOfCharacters ) const;

  JavaString ReplaceAll( char16_t param1, char16_t param2 ) const;

  bool Contains( const char16_t *pString ) const;
  bool Equals( const char16_t *pString ) const;

  size_t Hash() const;

  static size_t NotFound();

public:
  static const JavaString EmptyString();

private:
  void InternalCopyOtherValue( const uint8_t * pBuffer, size_t length );
  void InternalCleanup();

  // In order to conform with the general C++ standards, this method name is intentionally all lower case.
  void swap( JavaString &left, JavaString &right ) JVMX_NOEXCEPT;

private:
  boost::flyweight<std::u16string> m_Data;
};

// Create a specialization for this class in the standard namespace. This means that all unordered containers will use this function for hashing.
namespace std
{
  template<> class hash < JavaString >
  {
  public:
    std::size_t operator()( JavaString const& string ) const
    {
      return string.Hash();
    }
  };
}

#endif // __JAVASTRING_H__
