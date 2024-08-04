#ifndef _TYPEPARSER__H_
#define _TYPEPARSER__H_

#include <memory>
#include <vector>

#include "JavaArrayTypes.h"
#include "JavaString.h"
#include "JavaInteger.h"

#include "IMemoryManager.h"

const JVMX_CHAR_TYPE c_JavaTypeSpecifierVoid = JVMX_T( 'V' );
const JVMX_CHAR_TYPE c_JavaTypeSpecifierLong = JVMX_T( 'J' );
const JVMX_CHAR_TYPE c_JavaTypeSpecifierByte = JVMX_T( 'B' );
const JVMX_CHAR_TYPE c_JavaTypeSpecifierInteger = JVMX_T( 'I' );
const JVMX_CHAR_TYPE c_JavaTypeSpecifierChar = JVMX_T( 'C' );
const JVMX_CHAR_TYPE c_JavaTypeSpecifierBool = JVMX_T( 'Z' );
const JVMX_CHAR_TYPE c_JavaTypeSpecifierShort = JVMX_T( 'S' );
const JVMX_CHAR_TYPE c_JavaTypeSpecifierReference = JVMX_T( 'L' );
const JVMX_CHAR_TYPE c_JavaTypeSpecifierFloat = JVMX_T( 'F' );
const JVMX_CHAR_TYPE c_JavaTypeSpecifierDouble = JVMX_T( 'D' );
const JVMX_CHAR_TYPE c_JavaTypeSpecifierArray = JVMX_T( '[' );

const JVMX_CHAR_TYPE c_JavaObjectTypeDelimiter = JVMX_T( ';' );

// JVMX_ABSTRACT
class TypeParser 
{
public:
  struct ParsedMethodType
  {
  public:
    ParsedMethodType();
    size_t GetParameterCountAsIntegers() const;

    std::vector<boost::intrusive_ptr<JavaString> > parameters;
    JavaString returnType;
  };

public:
  static ParsedMethodType ParseMethodType( const JavaString &type );
  static std::vector<boost::intrusive_ptr<JavaString> > ParseParameterList( const JavaString &params );
  static JavaString ConvertTypeToString( e_JavaVariableTypes type );
  static JavaString ConvertTypeToString( JVMX_CHAR_TYPE type );
  static e_JavaVariableTypes ConvertTypeDescriptorToVariableType( char16_t type );
  static e_JavaArrayTypes ConvertTypeDescriptorToArrayType( char16_t type );
  static JavaString ExtractClassNameFromReference( const JavaString &referenceName );
  static e_JavaArrayTypes ExtractContainedTypeFromArrayTypeDescriptor( const JavaString &referenceName );

  static bool IsPrimitiveTypeDescriptor( char16_t type );

  static bool IsReferenceTypeDescriptor( JavaString descriptor );
  static bool IsReferenceTypeDescriptor( char16_t type );

  static bool IsArrayTypeDescriptor( JavaString type );

  static boost::intrusive_ptr<IJavaVariableType> GetDefaultValue( const JavaString &descriptor );
  static boost::intrusive_ptr<IJavaVariableType> GetDefaultValue( e_JavaArrayTypes type );

  static boost::intrusive_ptr<IJavaVariableType> DownCastFromInteger( boost::intrusive_ptr< JavaInteger > pIntegerValue, e_JavaVariableTypes requiredType );
  static boost::intrusive_ptr< JavaInteger > UpCastToInteger( const IJavaVariableType *pValue );
};

#endif // _TYPEPARSER__H_
