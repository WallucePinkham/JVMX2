
#include "ConstantPool.h"

#include "JavaObject.h"
#include "JavaClass.h"

#include "ConstantPoolStringReference.h"

ConstantPoolStringReference::ConstantPoolStringReference( ConstantPoolIndex index )
  : m_Value( index )
  , m_pString( nullptr )
{}

ConstantPoolStringReference::ConstantPoolStringReference( const ConstantPoolStringReference &other )
  : m_Value( other.m_Value )
  , m_pString( other.m_pString )
{
}

ConstantPoolStringReference::~ConstantPoolStringReference()
{}

ConstantPoolIndex ConstantPoolStringReference::ToConstantPoolIndex() const
{
  return m_Value;
}

std::shared_ptr<ConstantPoolStringReference> ConstantPoolStringReference::FromConstantPoolIndex( ConstantPoolIndex index )
{
  return std::shared_ptr< ConstantPoolStringReference >( new ConstantPoolStringReference( index ) );
}

ConstantPoolStringReference &ConstantPoolStringReference::operator=( const ConstantPoolStringReference &other )
{
  m_Value = other.m_Value;
  m_pString = other.m_pString;
  return *this;
}

bool ConstantPoolStringReference::operator==( const ConstantPoolStringReference &other ) const
{
  return m_Value == other.m_Value;
}

void ConstantPoolStringReference::Prepare( const ConstantPool *pPool )
{
  m_pString = pPool->GetConstant( m_Value )->AsString();
}

boost::intrusive_ptr<JavaString> ConstantPoolStringReference::GetStringValue() const
{
  // Copy the string to preserve const-ness.
  return new JavaString( *m_pString );
}


