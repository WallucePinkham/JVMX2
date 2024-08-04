

#include <cstring>

#include "GlobalConstants.h"

#include "JavaString.h"
#include "InvalidArgumentException.h"

#include "Stream.h"

#include "CodeAttributeUnknown.h"

CodeAttributeUnknown::CodeAttributeUnknown( const JavaString &name, Stream &byteStream, size_t byteLength ) :
JavaCodeAttribute( name, e_JavaAttributeTypeUnknown )
{
  m_Data = m_Data.Append( byteStream.ReadBytes( byteLength ) );
}

CodeAttributeUnknown::CodeAttributeUnknown( const CodeAttributeUnknown &other ) :
JavaCodeAttribute( other.m_Name, e_JavaAttributeTypeUnknown )
, JavaAttributeUnknown( other.m_Data.GetByteLength(), other.m_Data.ToByteArray() )
{}

CodeAttributeUnknown::CodeAttributeUnknown( CodeAttributeUnknown &&other ) :
JavaCodeAttribute( JavaString::EmptyString(), e_JavaAttributeTypeUnknown )
, JavaAttributeUnknown( 0, reinterpret_cast<const uint8_t *>("") )
{
  swap( *this, other );
}

CodeAttributeUnknown::~CodeAttributeUnknown() JVMX_NOEXCEPT
{
}

CodeAttributeUnknown &CodeAttributeUnknown::operator=(CodeAttributeUnknown other)
{
  swap( *this, other );
  return *this;
}


CodeAttributeUnknown CodeAttributeUnknown::FromBinary( JavaString name, Stream &byteStream, size_t byteCount )
{
  return CodeAttributeUnknown( name, byteStream, byteCount );
}

void CodeAttributeUnknown::swap( CodeAttributeUnknown &left, CodeAttributeUnknown &right ) JVMX_NOEXCEPT
{
  JavaCodeAttribute::swap( left, right );
  JavaAttributeUnknown::swap( left, right );
}

DataBuffer CodeAttributeUnknown::ToBinary( const ConstantPool & ) const
{
  return m_Data;
}

bool CodeAttributeUnknown::Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT
{
  if ( other.GetType() != m_Type )
  {
    return false;
  }

  return Equals( static_cast<const CodeAttributeUnknown &>(other) );
}

bool CodeAttributeUnknown::Equals( const CodeAttributeUnknown &other ) const JVMX_NOEXCEPT
{
  return m_Data == other.m_Data;
}

