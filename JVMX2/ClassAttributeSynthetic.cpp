
#include "ClassAttributeSynthetic.h"


ClassAttributeSynthetic::ClassAttributeSynthetic( const JavaString &name ) : JavaCodeAttribute( name, e_JavaAttributeTypeSynthetic )
{
}

ClassAttributeSynthetic::ClassAttributeSynthetic( const ClassAttributeSynthetic &other ) : JavaCodeAttribute( other.m_Name, e_JavaAttributeTypeSynthetic )
{
}

bool ClassAttributeSynthetic::Equals( const JavaCodeAttribute &other ) const
{
  if ( other.GetType() != m_Type )
  {
    return false;
  }

  return Equals( reinterpret_cast<const ClassAttributeSynthetic &>(other) );
}

bool ClassAttributeSynthetic::Equals( const ClassAttributeSynthetic &other ) const JVMX_NOEXCEPT
{
  return m_Name == other.m_Name &&
  m_Type == other.m_Type;
}

ClassAttributeSynthetic::~ClassAttributeSynthetic() JVMX_NOEXCEPT
{
}

ClassAttributeSynthetic ClassAttributeSynthetic::FromBinary( JavaString name )
{
  return ClassAttributeSynthetic( name );
}

DataBuffer ClassAttributeSynthetic::ToBinary( const ConstantPool & ) const
{
  return DataBuffer::EmptyBuffer();
}

