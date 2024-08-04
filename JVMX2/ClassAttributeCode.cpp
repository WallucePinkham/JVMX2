
#include "Stream.h"

#include "JavaCodeAttribute.h"
#include "ClassAttributeCode.h"


const JavaString c_AttributeCodeName = JavaString::FromCString( JVMX_T( "Code" ) );

ClassAttributeCode::ClassAttributeCode( const ClassAttributeCode &other ) :
JavaCodeAttribute( other )
, m_Code( other.m_Code )
, m_MaximumOperandStackDepth( other.m_MaximumOperandStackDepth )
, m_LocalVariableArraySizeIncludingPassedParameters( other.m_LocalVariableArraySizeIncludingPassedParameters )
, m_ExceptionTable( other.m_ExceptionTable )
, m_Attributes( other.m_Attributes )
{}

ClassAttributeCode::ClassAttributeCode( uint16_t maximumOperandStackDepth, uint16_t localVariableArraySizeIncludingPassedParameters, DataBuffer code, ExceptionTable exceptionTable, CodeAttributeList attributes ) :
JavaCodeAttribute( c_AttributeCodeName, e_JavaAttributeTypeCode )
, m_Code( code )
, m_MaximumOperandStackDepth( maximumOperandStackDepth )
, m_LocalVariableArraySizeIncludingPassedParameters( localVariableArraySizeIncludingPassedParameters )
, m_ExceptionTable( std::move( exceptionTable ) )
, m_Attributes( std::move( attributes ) )
{}

ClassAttributeCode::ClassAttributeCode( ClassAttributeCode &&other ) :
JavaCodeAttribute( c_AttributeCodeName, e_JavaAttributeTypeCode )
, m_Code( DataBuffer::EmptyBuffer() )
{
  swap( *this, other );
}

ClassAttributeCode::~ClassAttributeCode()
{}

ClassAttributeCode ClassAttributeCode::FromBinary( Stream &byteStream, CodeAttributeFactory &attributeFactory, const ConstantPool &constantPool )
{
  uint16_t maximumOperandStackDepth = byteStream.ReadUint16();
  uint16_t localVariableArraySizeIncludingPassedParameters = byteStream.ReadUint16();

  uint32_t codeLength = byteStream.ReadUint32();
  DataBuffer code = DataBuffer( byteStream.ReadBytes( codeLength ) );

  uint16_t exceptionTableLength = byteStream.ReadUint16();
  ExceptionTable exceptionTable;

  exceptionTable.reserve( exceptionTableLength );

  for ( uint16_t i = 0; i < exceptionTableLength; ++ i )
  {
    uint16_t startPos = byteStream.ReadUint16();
    uint16_t endPos = byteStream.ReadUint16();
    uint16_t handlerPos = byteStream.ReadUint16();
    ConstantPoolIndex handlerIndex = static_cast<ConstantPoolIndex>(byteStream.ReadUint16());

    std::shared_ptr<ConstantPoolClassReference> pHandlerReference = nullptr;
    if ( 0 != handlerIndex )
    {
      pHandlerReference = constantPool.GetConstant( handlerIndex )->AsClassReferencePointer();
    }

    exceptionTable.push_back( ExceptionTableEntry( startPos, endPos, handlerPos, pHandlerReference ) );
  }

  uint16_t attributesCount = byteStream.ReadUint16();
  CodeAttributeList attributes;
  attributes.reserve( attributesCount );

  for ( uint16_t i = 0; i < attributesCount; ++ i )
  {
    ConstantPoolIndex attributeNameIndex = byteStream.ReadUint16();
    JavaString attributeName = *constantPool.GetConstant( attributeNameIndex )->AsString();

    uint32_t attributeLength = byteStream.ReadUint32();

    std::shared_ptr<JavaCodeAttribute> pAttribute = attributeFactory.CreateFromBinary( attributeName, byteStream, constantPool, attributeLength );
    attributes.push_back( pAttribute );
  }

  return ClassAttributeCode( maximumOperandStackDepth, localVariableArraySizeIncludingPassedParameters, code, exceptionTable, attributes );
}

DataBuffer ClassAttributeCode::ToBinary( const ConstantPool &constantPool ) const
{
  DataBuffer result = DataBuffer::EmptyBuffer();
  result = result.AppendUint16( m_MaximumOperandStackDepth );
  result = result.AppendUint16( m_LocalVariableArraySizeIncludingPassedParameters );

  result = result.AppendUint32( static_cast<uint16_t>(m_Code.GetByteLength()) );
  result = result.Append( m_Code );

  result = result.AppendUint16( static_cast<uint16_t>(m_ExceptionTable.size()) );
  for (ExceptionTableEntry exception : m_ExceptionTable)
  {
    result = result.AppendUint16( exception.GetStartPosition() );
    result = result.AppendUint16( exception.GetEndPosition() );
    result = result.AppendUint16( exception.GetHandlerPosition() );

    result = result.AppendUint16( static_cast<uint16_t>(exception.GetCatchType()->ToConstantPoolIndex()) );
  }

  result = result.AppendUint16( static_cast<uint16_t>(m_Attributes.size()) );
  for (std::shared_ptr<JavaCodeAttribute> attribute : m_Attributes)
  {
    ConstantPool pool;
    DataBuffer attributeBinary = attribute->ToBinary( pool );

    result = result.AppendUint16( constantPool.GetStringConstantIndex( attribute->GetName() ) );
    result = result.AppendUint32( attributeBinary.GetByteLength() );

    result = result.Append( attributeBinary );
  }

  return result;
}

void ClassAttributeCode::swap( ClassAttributeCode &left, ClassAttributeCode &right ) JVMX_NOEXCEPT
{
  JavaCodeAttribute::swap( left, right );

  DataBuffer::swap( left.m_Code, right.m_Code );

  std::swap( left.m_Attributes, right.m_Attributes );
  std::swap( left.m_ExceptionTable, right.m_ExceptionTable );
  std::swap( left.m_MaximumOperandStackDepth, right.m_MaximumOperandStackDepth );
  std::swap( left.m_LocalVariableArraySizeIncludingPassedParameters, right.m_LocalVariableArraySizeIncludingPassedParameters );
}

const CodeAttributeList &ClassAttributeCode::GetAttributeList() const
{
  return m_Attributes;
}

const ExceptionTable & ClassAttributeCode::GetExceptionTable() const
{
  return m_ExceptionTable;
}

const DataBuffer & ClassAttributeCode::GetCode() const
{
  return m_Code;
}

uint16_t ClassAttributeCode::GetLocalVariableArraySizeIncludingPassedParameters() const
{
  return m_LocalVariableArraySizeIncludingPassedParameters;
}

uint16_t ClassAttributeCode::GetMaximumOperandStackDepth() const
{
  return m_MaximumOperandStackDepth;
}

bool ClassAttributeCode::Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT
{
  if ( other.GetType() != m_Type )
  {
    return false;
  }

  return Equals( static_cast<const ClassAttributeCode &>(other) );
}

bool ClassAttributeCode::Equals( const ClassAttributeCode &other ) const JVMX_NOEXCEPT
{
  return m_MaximumOperandStackDepth == other.m_MaximumOperandStackDepth &&
  m_LocalVariableArraySizeIncludingPassedParameters == other.m_LocalVariableArraySizeIncludingPassedParameters &&
  m_Code == other.m_Code &&
  std::equal( m_Attributes.begin(), m_Attributes.end(), other.m_Attributes.begin(), []( std::shared_ptr<JavaCodeAttribute> first, std::shared_ptr<JavaCodeAttribute> second ) { return *first == *second; } ) &&
  std::equal( m_ExceptionTable.begin(), m_ExceptionTable.end(), other.m_ExceptionTable.begin() );
}

