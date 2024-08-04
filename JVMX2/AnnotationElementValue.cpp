
#include <vector>
#include <memory>
#include <stdexcept>

#include "GlobalConstants.h"
#include "Stream.h"

#include "InvalidArgumentException.h"
#include "InvalidStateException.h"
#include "IndexOutOfBoundsException.h"
#include "UnsupportedOperationException.h"

#include "AnnotationElementValue.h"
#include "AnnotationsEntry.h"

AnnotationsElementValue::AnnotationsElementValue( const AnnotationsElementValue &other )
  : m_Type( other.m_Type )
  , m_ConstValueIndex( other.m_ConstValueIndex )
  , m_EnumConstValueTypeNameIndex( other.m_EnumConstValueTypeNameIndex )
  , m_EnumConstValueConstNameIndex( other.m_EnumConstValueConstNameIndex )
  , m_ClassInfoIndex( other.m_ClassInfoIndex )
  , m_pAnnotation( std::make_shared<AnnotationsEntry>( *other.m_pAnnotation ) )
{
  for (std::shared_ptr<AnnotationsElementValue> pAnnotationElementValue : m_Values)
  {
    std::shared_ptr<AnnotationsElementValue> pValue = std::make_shared<AnnotationsElementValue>( *pAnnotationElementValue );
    m_Values.push_back( pValue );
  }
}

AnnotationsElementValue::AnnotationsElementValue( AnnotationsElementValue &&other )
{
  swap( *this, other );
}

AnnotationsElementValue::AnnotationsElementValue()
  : m_Type( e_AnnotationsElementTypeUnset )
  , m_ConstValueIndex( 0 )
  , m_EnumConstValueTypeNameIndex( 0 )
  , m_EnumConstValueConstNameIndex( 0 )
  , m_ClassInfoIndex( 0 )
{}

AnnotationsElementValue::~AnnotationsElementValue() JVMX_NOEXCEPT
{
}

std::shared_ptr<AnnotationsElementValue> AnnotationsElementValue::FromBinary( Stream &stream )
{
  // Can't use make_shared here because the constructor is private :(
  std::shared_ptr<AnnotationsElementValue> pResult = std::shared_ptr<AnnotationsElementValue>( new AnnotationsElementValue() );

  uint8_t tag = stream.ReadUint8();
  e_AnnotationsElementTypes type = pResult->SetTypeFromTag( tag );

  switch ( type )
  {
    case e_AnnotationsElementTypeSignedByte:
    case e_AnnotationsElementTypeUnicodeChar:
    case e_AnnotationsElementTypeDouble:
    case e_AnnotationsElementTypeFloat:
    case e_AnnotationsElementTypeInt:
    case e_AnnotationsElementTypeLong:
    case e_AnnotationsElementTypeShort:
    case e_AnnotationsElementTypeBoolean:
    case e_AnnotationsElementTypeString:
      pResult->m_ConstValueIndex = stream.ReadUint16();
      break;

    case e_AnnotationsElementTypeEnumConstant:
      pResult->m_EnumConstValueTypeNameIndex = stream.ReadUint16();
      pResult->m_EnumConstValueConstNameIndex = stream.ReadUint16();
      break;

    case e_AnnotationsElementTypeReferenceToClass:
      pResult->m_ClassInfoIndex = stream.ReadUint16();
      break;

    case e_AnnotationsElementTypeAnnotation:
      pResult->m_pAnnotation = AnnotationsEntry::FromBinary( stream );
      break;

    case e_AnnotationsElementTypeArray:
    {
      uint16_t numValues = stream.ReadUint16();
      for ( uint16_t index = 0; index < numValues; ++ index )
      {
        pResult->m_Values.push_back( AnnotationsElementValue::FromBinary( stream ) );
      }
    }
    break;

    default:
      throw InvalidStateException( __FUNCTION__ " - Class is in invalid state. Unknown type." );
      break;
  }

  return pResult;
}

DataBuffer AnnotationsElementValue::ToBinary() const
{
  DataBuffer result = DataBuffer::EmptyBuffer();

  result = result.AppendUint8( GetTagFromType() );

  switch ( m_Type )
  {
    case e_AnnotationsElementTypeSignedByte:
    case e_AnnotationsElementTypeUnicodeChar:
    case e_AnnotationsElementTypeDouble:
    case e_AnnotationsElementTypeFloat:
    case e_AnnotationsElementTypeInt:
    case e_AnnotationsElementTypeLong:
    case e_AnnotationsElementTypeShort:
    case e_AnnotationsElementTypeBoolean:
    case e_AnnotationsElementTypeString:
      result = result.AppendUint16( m_ConstValueIndex );
      break;

    case e_AnnotationsElementTypeEnumConstant:
      result = result.AppendUint16( m_EnumConstValueTypeNameIndex );
      result = result.AppendUint16( m_EnumConstValueConstNameIndex );
      break;

    case e_AnnotationsElementTypeReferenceToClass:
      result = result.AppendUint16( m_ClassInfoIndex );
      break;

    case e_AnnotationsElementTypeAnnotation:
      result = result.Append( m_pAnnotation->ToBinary() );
      break;

    case e_AnnotationsElementTypeArray:
    {
      result = result.AppendUint16( static_cast<uint16_t>(m_Values.size()) );

      for ( uint16_t index = 0; index < m_Values.size(); ++ index )
      {
        result = result.Append( m_Values.at( index )->ToBinary() );
      }
    }
    break;

    default:
      throw InvalidStateException( __FUNCTION__ " - Class is in invalid state. Unknown type." );
      break;
  }

  return result;
}

e_AnnotationsElementTypes AnnotationsElementValue::GetType() const JVMX_NOEXCEPT
{
  return m_Type;
}

AnnotationsElementValue &AnnotationsElementValue::operator=(AnnotationsElementValue other)
{
  swap( *this, other );
  return *this;
}

bool AnnotationsElementValue::operator==(const AnnotationsElementValue &other) const JVMX_NOEXCEPT
{
  if ( m_Type != other.m_Type )
  {
    return false;
  }

  if ( m_ConstValueIndex != other.m_ConstValueIndex )
  {
    return false;
  }

  if ( m_EnumConstValueConstNameIndex != other.m_EnumConstValueConstNameIndex )
  {
    return false;
  }

  if ( m_EnumConstValueConstNameIndex != other.m_EnumConstValueConstNameIndex )
  {
    return false;
  }

  if ( m_ClassInfoIndex != other.m_ClassInfoIndex )
  {
    return false;
  }

  if ( nullptr != m_pAnnotation && nullptr == other.m_pAnnotation )
  {
    return false;
  }

  if ( nullptr == m_pAnnotation && nullptr != other.m_pAnnotation )
  {
    return false;
  }

  if ( nullptr != m_pAnnotation && nullptr != other.m_pAnnotation )
  {
    if ( !(*m_pAnnotation == *other.m_pAnnotation) )
    {
      return false;
    }
  }

  return std::equal( m_Values.cbegin(), m_Values.cend(), other.m_Values.cbegin(), []( std::shared_ptr<AnnotationsElementValue> p1, std::shared_ptr<AnnotationsElementValue> p2 ){ return *p1 == *p2; } );
}


ConstantPoolIndex AnnotationsElementValue::GetEnumConstValueTypeNameIndex() const
{
  if ( e_AnnotationsElementTypeEnumConstant != m_Type )
  {
    throw UnsupportedOperationException( __FUNCTION__ " - Trying to get an enum value when this is not an enum type." );
  }

  return m_EnumConstValueTypeNameIndex;
}

ConstantPoolIndex AnnotationsElementValue::GetEnumConstValueConstNameIndex() const
{
  if ( e_AnnotationsElementTypeEnumConstant != m_Type )
  {
    throw UnsupportedOperationException( __FUNCTION__ " - Trying to get an enum value when this is not an enum type." );
  }

  return m_EnumConstValueConstNameIndex;
}

ConstantPoolIndex AnnotationsElementValue::GetClassInfoIndex() const
{
  if ( e_AnnotationsElementTypeReferenceToClass != m_Type )
  {
    throw UnsupportedOperationException( __FUNCTION__ " - Trying to get an class index when this is not an class type." );
  }

  return m_ClassInfoIndex;
}

std::shared_ptr<AnnotationsEntry> AnnotationsElementValue::GetNestedAnnotation() const
{
  if ( e_AnnotationsElementTypeAnnotation != m_Type )
  {
    throw UnsupportedOperationException( __FUNCTION__ " - Trying to get an annotation when this is not an annotation type." );
  }

  return m_pAnnotation;
}

uint16_t AnnotationsElementValue::GetArrayValueNumberOfValues() const
{
  if ( e_AnnotationsElementTypeArray != m_Type )
  {
    throw UnsupportedOperationException( __FUNCTION__ " - Trying to get an array length when this is not an array type." );
  }

  return static_cast<uint16_t>(m_Values.size());
}

std::shared_ptr<AnnotationsElementValue> AnnotationsElementValue::GetArrayValueAt( size_t index )
{
  if ( e_AnnotationsElementTypeArray != m_Type )
  {
    throw UnsupportedOperationException( __FUNCTION__ " - Trying to get an array value when this is not an array type." );
  }

  try
  {
    return m_Values.at( index );
  }
  catch ( std::out_of_range& )
  {
    throw IndexOutOfBoundsException( __FUNCTION__ " - Could not get Method name. Index out of bounds." );
  }
}

ConstantPoolIndex AnnotationsElementValue::GetConstValueIndex() const
{
  switch ( m_Type )
  {
    case e_AnnotationsElementTypeSignedByte:
    case e_AnnotationsElementTypeUnicodeChar:
    case e_AnnotationsElementTypeDouble:
    case e_AnnotationsElementTypeFloat:
    case e_AnnotationsElementTypeInt:
    case e_AnnotationsElementTypeLong:
    case e_AnnotationsElementTypeShort:
    case e_AnnotationsElementTypeBoolean:
    case e_AnnotationsElementTypeString:
      break;

    default:
      throw UnsupportedOperationException( __FUNCTION__ " - Trying to get an base type value when this is not a base type." );
  }

  return m_ConstValueIndex;
}


void AnnotationsElementValue::swap( AnnotationsElementValue &left, AnnotationsElementValue &right ) JVMX_NOEXCEPT
{
  std::swap( left.m_Type, right.m_Type );
  std::swap( left.m_ConstValueIndex, right.m_ConstValueIndex );
  std::swap( left.m_EnumConstValueTypeNameIndex, right.m_EnumConstValueTypeNameIndex );
  std::swap( left.m_EnumConstValueConstNameIndex, right.m_EnumConstValueConstNameIndex );
  std::swap( left.m_ClassInfoIndex, right.m_ClassInfoIndex );
  std::swap( left.m_pAnnotation, right.m_pAnnotation );
  std::swap( left.m_Values, right.m_Values );
}

e_AnnotationsElementTypes AnnotationsElementValue::SetTypeFromTag( uint8_t tag )
{
  switch ( tag )
  {
    case 'B':
      m_Type = e_AnnotationsElementTypeSignedByte;
      break;

    case 'C':
      m_Type = e_AnnotationsElementTypeUnicodeChar;
      break;

    case 'D':
      m_Type = e_AnnotationsElementTypeDouble;
      break;

    case 'F':
      m_Type = e_AnnotationsElementTypeFloat;
      break;

    case 'I':
      m_Type = e_AnnotationsElementTypeInt;
      break;

    case 'J':
      m_Type = e_AnnotationsElementTypeLong;
      break;

    case 'S':
      m_Type = e_AnnotationsElementTypeShort;
      break;

    case 'Z':
      m_Type = e_AnnotationsElementTypeBoolean;
      break;

    case 's':
      m_Type = e_AnnotationsElementTypeString;
      break;

    case 'e':
      m_Type = e_AnnotationsElementTypeEnumConstant;
      break;

    case 'c':
      m_Type = e_AnnotationsElementTypeReferenceToClass;
      break;

    case '@':
      m_Type = e_AnnotationsElementTypeAnnotation;
      break;

    case '[':
      m_Type = e_AnnotationsElementTypeArray;
      break;

    default:
      throw InvalidArgumentException( __FUNCTION__ " - Unknown tag passed in." );
  }

  return m_Type;
}

uint8_t AnnotationsElementValue::GetTagFromType() const
{
  switch ( m_Type )
  {
    case e_AnnotationsElementTypeSignedByte:
      return 'B';
      break;

    case e_AnnotationsElementTypeUnicodeChar:
      return 'C';
      break;

    case e_AnnotationsElementTypeDouble:
      return 'D';
      break;

    case e_AnnotationsElementTypeFloat:
      return 'F';
      break;

    case e_AnnotationsElementTypeInt:
      return 'I';
      break;

    case e_AnnotationsElementTypeLong:
      return 'J';
      break;

    case e_AnnotationsElementTypeShort:
      return 'S';
      break;

    case e_AnnotationsElementTypeBoolean:
      return 'Z';
      break;

    case e_AnnotationsElementTypeString:
      return 's';
      break;

    case e_AnnotationsElementTypeEnumConstant:
      return 'e';
      break;

    case e_AnnotationsElementTypeReferenceToClass:
      return 'c';
      break;

    case e_AnnotationsElementTypeAnnotation:
      return '@';
      break;

    case e_AnnotationsElementTypeArray:
      return '[';
      break;

    default:
      throw InvalidArgumentException( __FUNCTION__ " - Unknown type value in class." );
  }
}

//////////////////////////////////////////////////////////////////////////
/// AnnotationsElementValuePair Methods
//////////////////////////////////////////////////////////////////////////

AnnotationsElementValuePair::AnnotationsElementValuePair( ConstantPoolIndex elementNameIndex, std::shared_ptr<AnnotationsElementValue> &pElementValue )
  : m_ElementNameIndex( elementNameIndex )
  , m_pElementValue( pElementValue )
{}

bool AnnotationsElementValuePair::operator==(const AnnotationsElementValuePair &other) const
{
  return m_ElementNameIndex == other.m_ElementNameIndex &&
    *m_pElementValue == *other.m_pElementValue;
}

AnnotationsElementValuePair::~AnnotationsElementValuePair() JVMX_NOEXCEPT
{
}
