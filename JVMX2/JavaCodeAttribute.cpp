

#include "TypeMismatchException.h"

#include "AttributeConstantValue.h"
#include "CodeAttributeUnknown.h"
#include "ClassAttributeCode.h"

#include "JavaCodeAttribute.h"


JavaCodeAttribute::JavaCodeAttribute( const JavaString &name, e_JavaAttributeTypes type ) JVMX_NOEXCEPT :
m_Name( name )
, m_Type( type )
{
}

JavaCodeAttribute::JavaCodeAttribute( const JavaCodeAttribute &other ) :
m_Type( other.m_Type )
, m_Name( other.m_Name )
{}

JavaCodeAttribute::JavaCodeAttribute( JavaCodeAttribute &&other ) :
m_Type( std::move( other.m_Type ) )
, m_Name( std::move( other.m_Name ) )
{}

JavaCodeAttribute::~JavaCodeAttribute() JVMX_NOEXCEPT
{
}

JavaString JavaCodeAttribute::GetName() const
{
  return m_Name;
}

e_JavaAttributeTypes JavaCodeAttribute::GetType() const
{
  return m_Type;
}

void JavaCodeAttribute::swap( JavaCodeAttribute &left, JavaCodeAttribute &right ) JVMX_NOEXCEPT
{
  std::swap( left.m_Name, right.m_Name );
  std::swap( left.m_Type, right.m_Type );
}

// JavaCodeAttribute &JavaCodeAttribute::operator=( JavaCodeAttribute other )
// {
//   swap( *this, other );
//   return *this;
// }


CodeAttributeUnknown JavaCodeAttribute::ToUnknown() const
{
  if ( e_JavaAttributeTypeUnknown != m_Type )
  {
    throw TypeMismatchException( __FUNCTION__ " - Tried to convert a attribute to another type. Internal Types didn't match." );
  }

  const CodeAttributeUnknown *pResult = dynamic_cast<const CodeAttributeUnknown *>(this);

  if ( nullptr == pResult )
  {
    throw TypeMismatchException( __FUNCTION__ " - Tried to convert a attribute to another type. Dynamic cast failed." );
  }

  return CodeAttributeUnknown( *pResult );
}

AttributeConstantValue JavaCodeAttribute::ToConstantValue() const
{
  if ( e_JavaAttributeTypeConstantValue != m_Type )
  {
    throw TypeMismatchException( __FUNCTION__ " - Tried to convert a attribute to another type. Internal Types didn't match." );
  }

  const AttributeConstantValue *pResult = dynamic_cast<const AttributeConstantValue *>(this);

  if ( nullptr == pResult )
  {
    throw TypeMismatchException( __FUNCTION__ " - Tried to convert a attribute to another type. Dynamic cast failed." );
  }

  return AttributeConstantValue( *pResult );
}

ClassAttributeCode JavaCodeAttribute::ToCode() const
{
  if ( e_JavaAttributeTypeCode != m_Type )
  {
    throw TypeMismatchException( __FUNCTION__ " - Tried to convert a attribute to another type. Internal Types didn't match." );
  }

  const ClassAttributeCode *pResult = dynamic_cast<const ClassAttributeCode *>(this);

  if ( nullptr == pResult )
  {
    throw TypeMismatchException( __FUNCTION__ " - Tried to convert a attribute to another type. Dynamic cast failed." );
  }

  return ClassAttributeCode( *pResult );
}


bool JavaCodeAttribute::operator==(const JavaCodeAttribute &other) const
{
  if ( !m_Type == other.m_Type && m_Name == other.m_Name )
  {
    return false;
  }

  return Equals( other );
}

