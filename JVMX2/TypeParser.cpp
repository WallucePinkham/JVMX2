
#include "GlobalConstants.h"

#include "GlobalCatalog.h"
#include "IVirtualMachineState.h"
#include "ILogger.h"

#include "JavaTypes.h"

//#include "MallocFreeMemoryManager.h"
#include "UnsupportedTypeException.h"
#include "InvalidArgumentException.h"

#include "ObjectFactory.h"

#include "TypeParser.h"
#include "HelperTypes.h"

const JVMX_CHAR_TYPE c_JavaParametersStart = '(';
const JVMX_CHAR_TYPE c_JavaParametersEnd = ')';

TypeParser::ParsedMethodType::ParsedMethodType()
  : returnType( JavaString::EmptyString() )
{}

TypeParser::ParsedMethodType TypeParser::ParseMethodType( const JavaString &type )
{
  ParsedMethodType result;

  if ( type.At( 0 ) != c_JavaParametersStart )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Missing initial brace." );
  }

  size_t endParamBrace = type.FindNext( 0, c_JavaParametersEnd );
  if ( endParamBrace == type.GetLastStringPosition() )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Missing brace to end parameters." );
  }

  result.returnType = type.SubString( endParamBrace + 1, type.GetLastStringPosition() );
  result.parameters = ParseParameterList( type.SubString( 1, endParamBrace - 1 ) ); // -1 for the end brace.

  return result;
}

std::vector<boost::intrusive_ptr<JavaString> > TypeParser::ParseParameterList( const JavaString &params )
{
  size_t index = 0;
  //size_t lastIndex = 0;

  std::vector<boost::intrusive_ptr<JavaString> > result;

  while ( index < params.GetLengthInCodePoints() )
  {
    if ( c_JavaTypeSpecifierReference == params.At( index ) )
    {
      size_t delimiterPos = params.FindNext( index, c_JavaObjectTypeDelimiter );

      if ( delimiterPos != params.GetLastStringPosition() )
      {
        result.push_back( new JavaString( params.SubString( index, delimiterPos ) ) );
      }

      index = delimiterPos;
    }
    else if ( c_JavaTypeSpecifierArray == params.At( index ) )
    {
      size_t endPos = index + 1;
      while ( c_JavaTypeSpecifierArray == params.At( endPos ) )
      {
        ++ endPos;
      }

      if ( c_JavaTypeSpecifierReference == params.At( endPos ) )
      {
        size_t delimiterPos = params.FindNext( index, c_JavaObjectTypeDelimiter );

        if ( delimiterPos != params.GetLastStringPosition() )
        {
          endPos = delimiterPos;
        }

        result.push_back( new JavaString( params.SubString( index, endPos - index ) ) );
        ++endPos; // We need the contained type, also.
      }
      else
      {
        ++endPos; // We need the contained type, also.
        result.push_back( new JavaString( params.SubString( index, endPos - index ) ) );
      }

      index = endPos - 1;
    }
    else
    {
      result.push_back( new JavaString( params.SubString( index, 1 ) ) );
    }

    ++index;
  }

  return result;
}

JavaString TypeParser::ConvertTypeToString( e_JavaVariableTypes type )
{
  switch ( type )
  {
    case e_JavaVariableTypes::Char:
      return JavaString::FromCString( JVMX_T( "Char" ) );
      break;

    case e_JavaVariableTypes::Byte:
      return JavaString::FromCString( JVMX_T( "Byte" ) );
      break;

    case e_JavaVariableTypes::Short:
      return JavaString::FromCString( JVMX_T( "Short" ) );
      break;

    case e_JavaVariableTypes::Integer:
      return JavaString::FromCString( JVMX_T( "Integer" ) );
      break;

    case e_JavaVariableTypes::Long:
      return JavaString::FromCString( JVMX_T( "Long" ) );
      break;

    case e_JavaVariableTypes::Float:
      return JavaString::FromCString( JVMX_T( "Float" ) );
      break;

    case e_JavaVariableTypes::Double:
      return JavaString::FromCString( JVMX_T( "Double" ) );
      break;

    case e_JavaVariableTypes::String:
      return JavaString::FromCString( JVMX_T( "StringRef" ) );
      break;

    case e_JavaVariableTypes::NullReference:
      return JavaString::FromCString( JVMX_T( "NullReference" ) );
      break;

    case e_JavaVariableTypes::ReturnAddress:
      return JavaString::FromCString( JVMX_T( "ReturnAddress" ) );
      break;

    case e_JavaVariableTypes::ClassReference:
      return JavaString::FromCString( JVMX_T( "ClassReference" ) );
      break;

    case e_JavaVariableTypes::Bool:
      return JavaString::FromCString( JVMX_T( "Boolean" ) );
      break;

    case e_JavaVariableTypes::Array:
      return JavaString::FromCString( JVMX_T( "Array" ) );
      break;

    case e_JavaVariableTypes::Object:
      return JavaString::FromCString( JVMX_T( "Object" ) );
      break;

    default:
      throw InvalidArgumentException( __FUNCTION__ " - Unknown type. " );
      break;
  }
}

JavaString TypeParser::ConvertTypeToString( JVMX_CHAR_TYPE type )
{
  switch ( type )
  {
    case c_JavaTypeSpecifierChar:
      return JavaString::FromCString( JVMX_T( "Char" ) );
      break;

    case c_JavaTypeSpecifierByte:
      return JavaString::FromCString( JVMX_T( "Byte" ) );
      break;

    case c_JavaTypeSpecifierShort:
      return JavaString::FromCString( JVMX_T( "Short" ) );
      break;

    case c_JavaTypeSpecifierInteger:
      return JavaString::FromCString( JVMX_T( "Integer" ) );
      break;

    case c_JavaTypeSpecifierLong:
      return JavaString::FromCString( JVMX_T( "Long" ) );
      break;

    case c_JavaTypeSpecifierFloat:
      return JavaString::FromCString( JVMX_T( "Float" ) );
      break;

    case c_JavaTypeSpecifierDouble:
      return JavaString::FromCString( JVMX_T( "Double" ) );
      break;

    case c_JavaTypeSpecifierReference:
      return JavaString::FromCString( JVMX_T( "Reference" ) );
      break;

    case c_JavaTypeSpecifierBool:
      return JavaString::FromCString( JVMX_T( "Boolean" ) );
      break;

    case c_JavaTypeSpecifierArray:
      return JavaString::FromCString( JVMX_T( "Array" ) );
      break;

    case c_JavaTypeSpecifierVoid:
      return JavaString::FromCString( JVMX_T( "Void" ) );
      break;

    default:
      throw InvalidArgumentException( __FUNCTION__ " - Unknown type. " );
      break;
  }
}

bool TypeParser::IsReferenceTypeDescriptor( JavaString descriptor )
{
  switch ( descriptor.At( 0 ) )
  {
    case c_JavaTypeSpecifierReference:
    case c_JavaTypeSpecifierArray:
      return true;

    default:
      break;
  }

  return false;
}

bool TypeParser::IsArrayTypeDescriptor( JavaString type )
{
  switch ( type.At( 0 ) )
  {
    case c_JavaTypeSpecifierArray:
      return true;

    default:
      break;
  }

  return false;
}

bool TypeParser::IsReferenceTypeDescriptor( char16_t descriptor )
{
  switch ( descriptor )
  {
    case c_JavaTypeSpecifierReference:
    case c_JavaTypeSpecifierArray:
      return true;
  }

  return false;
}

boost::intrusive_ptr<IJavaVariableType> TypeParser::GetDefaultValue( const JavaString &descriptor )
{
  boost::intrusive_ptr<IJavaVariableType> pDefaultValue;

  switch ( descriptor.At( 0 ) )
  {
    case c_JavaTypeSpecifierBool:
    {
      static const boost::intrusive_ptr<IJavaVariableType> pDefaultBool = new JavaBool( JavaBool::FromDefault() );
      pDefaultValue = pDefaultBool;
      break;
    }

    case c_JavaTypeSpecifierChar:
    {
      static const boost::intrusive_ptr<IJavaVariableType> pDefaultChar = new JavaChar( JavaChar::FromDefault() );
      pDefaultValue = pDefaultChar;
      break;
    }

    case c_JavaTypeSpecifierFloat:
    {
      static const boost::intrusive_ptr<IJavaVariableType> pDefaultFloat = new JavaFloat( JavaFloat::FromDefault() );
      pDefaultValue = pDefaultFloat;
      break;
    }

    case c_JavaTypeSpecifierDouble:
    {
      static const boost::intrusive_ptr<IJavaVariableType> pDefaultDouble = new JavaDouble( JavaDouble::FromDefault() );
      pDefaultValue = pDefaultDouble;
      break;
    }

    case c_JavaTypeSpecifierByte:
    {
      static const boost::intrusive_ptr<IJavaVariableType> pDefaultByte = new JavaByte( JavaByte::FromDefault() );
      pDefaultValue = pDefaultByte;
      break;
    }

    case c_JavaTypeSpecifierShort:
    {
      static const boost::intrusive_ptr<IJavaVariableType> pDefaultShort = new JavaShort( JavaShort::FromDefault() );
      pDefaultValue = pDefaultShort;
      break;
    }

    case c_JavaTypeSpecifierInteger:
    {
      static const boost::intrusive_ptr<IJavaVariableType> pDefaultInteger = new JavaInteger( JavaInteger::FromDefault() );
      pDefaultValue = pDefaultInteger;
      break;
    }

    case c_JavaTypeSpecifierLong:
    {
      static const boost::intrusive_ptr<IJavaVariableType> pDefaultLong = new JavaLong( JavaLong::FromDefault() );
      pDefaultValue = pDefaultLong;
      break;
    }

    case c_JavaTypeSpecifierArray:
    {
      static boost::intrusive_ptr<IJavaVariableType> pDefaultArray = nullptr;
      if ( nullptr == pDefaultArray )
      {
        /// *** Special case for creating arrays before the Thread Manager has been initialized :(
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
         std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
         pLogger->LogDebug( "Creating array of type: %c with size %d", static_cast<int>(descriptor.At( 1 )), static_cast<int>(0) );
 #endif // _DEBUG

        pDefaultArray = new ObjectReference( nullptr );
      }

      pDefaultValue = pDefaultArray;
      break;
    }

    case c_JavaTypeSpecifierReference:
    {
      static const boost::intrusive_ptr<IJavaVariableType> pDefaultNull = new ObjectReference(nullptr);
      pDefaultValue = pDefaultNull;
      break;
    }

    default:
      throw UnsupportedTypeException( __FUNCTION__ " - Unknown type. " );
      break;
  }

  return pDefaultValue;
}

e_JavaVariableTypes TypeParser::ConvertTypeDescriptorToVariableType( char16_t type )
{
  switch ( type )
  {
    case c_JavaTypeSpecifierBool:
      return e_JavaVariableTypes::Bool;
      break;

    case c_JavaTypeSpecifierChar:
      return e_JavaVariableTypes::Char;
      break;

    case c_JavaTypeSpecifierFloat:
      return e_JavaVariableTypes::Float;
      break;

    case c_JavaTypeSpecifierDouble:
      return e_JavaVariableTypes::Double;
      break;

    case c_JavaTypeSpecifierByte:
      return e_JavaVariableTypes::Byte;
      break;

    case c_JavaTypeSpecifierShort:
      return e_JavaVariableTypes::Short;
      break;

    case c_JavaTypeSpecifierInteger:
      return e_JavaVariableTypes::Integer;
      break;

    case c_JavaTypeSpecifierLong:
      return e_JavaVariableTypes::Long;
      break;

    case c_JavaTypeSpecifierArray:
      return e_JavaVariableTypes::Array;
      break;

    case c_JavaTypeSpecifierReference:
      return e_JavaVariableTypes::Object;
      break;

    default:
      throw InvalidArgumentException( __FUNCTION__ " - Unknown type. " );
      break;
  }
}

e_JavaArrayTypes TypeParser::ConvertTypeDescriptorToArrayType( char16_t type )
{
  switch ( type )
  {
    case c_JavaTypeSpecifierBool:
      return e_JavaArrayTypes::Boolean;
      break;

    case c_JavaTypeSpecifierChar:
      return e_JavaArrayTypes::Char;
      break;

    case c_JavaTypeSpecifierFloat:
      return e_JavaArrayTypes::Float;
      break;

    case c_JavaTypeSpecifierDouble:
      return e_JavaArrayTypes::Double;
      break;

    case c_JavaTypeSpecifierByte:
      return e_JavaArrayTypes::Byte;
      break;

    case c_JavaTypeSpecifierShort:
      return e_JavaArrayTypes::Short;
      break;

    case c_JavaTypeSpecifierInteger:
      return e_JavaArrayTypes::Integer;
      break;

    case c_JavaTypeSpecifierLong:
      return e_JavaArrayTypes::Long;
      break;

    case c_JavaTypeSpecifierArray:
      return e_JavaArrayTypes::Reference;
      break;

    case c_JavaTypeSpecifierReference:
      return e_JavaArrayTypes::Reference;
      break;

    default:
      throw InvalidArgumentException( __FUNCTION__ " - Unknown type. " );
      break;
  }
}

bool TypeParser::IsPrimitiveTypeDescriptor( char16_t type )
{
  switch ( type )
  {
    case c_JavaTypeSpecifierChar:
    case c_JavaTypeSpecifierByte:
    case c_JavaTypeSpecifierShort:
    case c_JavaTypeSpecifierInteger:
    case c_JavaTypeSpecifierLong:
    case c_JavaTypeSpecifierFloat:
    case c_JavaTypeSpecifierDouble:
    case c_JavaTypeSpecifierBool:
      return true;
  }

  return false;
}

JavaString ConvertArrayTypeToDescriptor( e_JavaArrayTypes type )
{
  switch ( type )
  {
    case e_JavaArrayTypes::Boolean:
      return JavaString::FromChar( c_JavaTypeSpecifierBool );
      break;

    case e_JavaArrayTypes::Char:
      return JavaString::FromChar( c_JavaTypeSpecifierChar );
      break;

    case e_JavaArrayTypes::Float:
      return JavaString::FromChar( c_JavaTypeSpecifierFloat );
      break;

    case e_JavaArrayTypes::Double:
      return JavaString::FromChar( c_JavaTypeSpecifierDouble );
      break;

    case e_JavaArrayTypes::Byte:
      return JavaString::FromChar( c_JavaTypeSpecifierByte );
      break;

    case e_JavaArrayTypes::Short:
      return JavaString::FromChar( c_JavaTypeSpecifierShort );
      break;

    case e_JavaArrayTypes::Integer:
      return JavaString::FromChar( c_JavaTypeSpecifierInteger );
      break;

    case e_JavaArrayTypes::Long:
      return JavaString::FromChar( c_JavaTypeSpecifierLong );
      break;

    case e_JavaArrayTypes::Reference:
      return JavaString::FromChar( c_JavaTypeSpecifierReference );
      break;

    default:
      throw InvalidArgumentException( __FUNCTION__ " - Unknown array type." );
      break;
  }
}

boost::intrusive_ptr<IJavaVariableType> TypeParser::GetDefaultValue( e_JavaArrayTypes type )
{
  JavaString descriptor = ConvertArrayTypeToDescriptor( type );

  return GetDefaultValue( descriptor );
}

boost::intrusive_ptr<IJavaVariableType> TypeParser::DownCastFromInteger( boost::intrusive_ptr< JavaInteger > pIntegerValue, e_JavaVariableTypes requiredType )
{
  switch ( requiredType )
  {
    case e_JavaVariableTypes::Char:
      return pIntegerValue->ToChar();
      break;

    case e_JavaVariableTypes::Byte:
      return pIntegerValue->ToByte();
      break;

    case e_JavaVariableTypes::Short:
      return pIntegerValue->ToShort();
      break;

    case e_JavaVariableTypes::Integer:
      return pIntegerValue;
      break;

    case e_JavaVariableTypes::Bool:
      return pIntegerValue->ToBool();
      break;

    default:
      throw InvalidArgumentException( __FUNCTION__ " - This type is not compatible with the integer type." );
      break;
  }

  return nullptr;
}

// boost::intrusive_ptr< JavaInteger > TypeParser::UpCastToInteger( boost::intrusive_ptr<IJavaVariableType> pValue )
// {
//   return UpCastToInteger( pValue.get() );
// }

boost::intrusive_ptr< JavaInteger > TypeParser::UpCastToInteger( const IJavaVariableType *pValue )
{
  switch ( pValue->GetVariableType() )
  {
    case e_JavaVariableTypes::Char:
      return new JavaInteger( JavaInteger::FromChar( *dynamic_cast<const JavaChar *>(pValue) ) );
      break;

    case e_JavaVariableTypes::Byte:
      return new JavaInteger( JavaInteger::FromByte( *dynamic_cast<const JavaByte *>(pValue) ) );
      break;

    case e_JavaVariableTypes::Short:
      return new JavaInteger( JavaInteger::FromShort( *dynamic_cast<const JavaShort *>(pValue) ) );
      break;

    case e_JavaVariableTypes::Integer:
      return new JavaInteger( *dynamic_cast<const JavaInteger *>(pValue) );
      break;

    case e_JavaVariableTypes::Bool:
      return new JavaInteger( JavaInteger::FromBool( *dynamic_cast<const JavaBool *>(pValue) ) );
      break;

    default:
      throw InvalidArgumentException( __FUNCTION__ " - This type is not compatible with the integer type." );
      break;
  }
}

JavaString TypeParser::ExtractClassNameFromReference( const JavaString &referenceName )
{
  size_t startIndex = 0;
  while ( referenceName.At( startIndex ) == c_JavaTypeSpecifierArray )
  {
    ++ startIndex;
  }

  if ( referenceName.At( startIndex ) != c_JavaTypeSpecifierReference )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected reference type to be passed in." );
  }

  size_t endPos = referenceName.FindNext( startIndex, ';' );
  if ( JavaString::NotFound() == endPos )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Semicolon at the end of reference was not found." );
  }

  return referenceName.SubString( 1, endPos - 1 );
}

e_JavaArrayTypes TypeParser::ExtractContainedTypeFromArrayTypeDescriptor( const JavaString &referenceName )
{
  size_t startIndex = 0;
  while ( referenceName.At( startIndex ) == c_JavaTypeSpecifierArray )
  {
    ++ startIndex;
  }

  return ConvertTypeDescriptorToArrayType( referenceName.At( startIndex ) );
}

size_t TypeParser::ParsedMethodType::GetParameterCountAsIntegers() const
{
  size_t size = 0;

  for ( auto it : parameters )
  {
    switch ( it->At( 0 ) )
    {
      case c_JavaTypeSpecifierBool:
        ++ size;
        break;

      case c_JavaTypeSpecifierChar:
        ++ size;
        break;

      case c_JavaTypeSpecifierFloat:
        ++ size;
        break;

      case c_JavaTypeSpecifierDouble:
        size += 2;
        break;

      case c_JavaTypeSpecifierByte:
        ++ size;
        break;

      case c_JavaTypeSpecifierShort:
        ++ size;
        break;

      case c_JavaTypeSpecifierInteger:
        ++ size;
        break;

      case c_JavaTypeSpecifierLong:
        size += 2;
        break;

      case c_JavaTypeSpecifierArray:
        ++ size;
        break;

      case c_JavaTypeSpecifierReference:
        ++ size;
        break;

      default:
        throw InvalidArgumentException( __FUNCTION__ " - Unknown type. " );
        break;
    }
  }

  return size;
}

