
#include "Stream.h"

#include "CodeAttributeDeprecated.h"

CodeAttributeDeprecated::CodeAttributeDeprecated( const CodeAttributeDeprecated &other )
  : JavaCodeAttribute( other )
{}

CodeAttributeDeprecated::CodeAttributeDeprecated( const JavaString &name )
  : JavaCodeAttribute( name, e_JavaAttributeTypeDeprecated )
{
}

CodeAttributeDeprecated::CodeAttributeDeprecated( CodeAttributeDeprecated &&other ) : JavaCodeAttribute( JavaString::EmptyString(), e_JavaAttributeTypeDeprecated )
{
  swap( *this, other );
}

CodeAttributeDeprecated::~CodeAttributeDeprecated()
{}

CodeAttributeDeprecated CodeAttributeDeprecated::FromBinary( JavaString name )
{
  return CodeAttributeDeprecated( name );
}

void CodeAttributeDeprecated::swap( CodeAttributeDeprecated &left, CodeAttributeDeprecated &right ) JVMX_NOEXCEPT
{
  JavaCodeAttribute::swap( left, right );
}

DataBuffer CodeAttributeDeprecated::ToBinary( const ConstantPool & ) const
{
  return DataBuffer::EmptyBuffer();
}

bool CodeAttributeDeprecated::Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT
{
  if ( other.GetType() != m_Type )
  {
    return false;
  }

  return Equals( static_cast<const CodeAttributeDeprecated &>(other) );
}

bool CodeAttributeDeprecated::Equals( const CodeAttributeDeprecated &other ) const JVMX_NOEXCEPT
{
  return GetType() == other.GetType();
}

