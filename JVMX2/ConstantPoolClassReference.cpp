#include "ConstantPool.h"

#include "JavaString.h"

#include "ConstantPoolClassReference.h"

ConstantPoolClassReference::ConstantPoolClassReference( uint16_t index )
  : m_Value( index )
  , m_pName( nullptr )
  , m_Prepared( false )
{}

ConstantPoolClassReference::ConstantPoolClassReference( const ConstantPoolClassReference &other )
  : m_Value( other.m_Value )
  , m_pName( other.m_pName )
  , m_Prepared( other.m_Prepared )
{}

ConstantPoolClassReference::~ConstantPoolClassReference()
{}

ConstantPoolIndex ConstantPoolClassReference::ToConstantPoolIndex() const
{
  return m_Value;
}

std::shared_ptr<ConstantPoolClassReference> ConstantPoolClassReference::FromConstantPoolIndex( ConstantPoolIndex index )
{
  return std::shared_ptr<ConstantPoolClassReference>( new ConstantPoolClassReference( index ) );
}

ConstantPoolClassReference& ConstantPoolClassReference::operator=( const ConstantPoolClassReference &other )
{
  if ( this != &other )
  {
    m_Value = other.m_Value;
    m_pName = other.m_pName;
    m_Prepared = other.m_Prepared;
  }

  return *this;
}

bool ConstantPoolClassReference::operator==( const ConstantPoolClassReference &other ) const
{
  return m_Value == other.m_Value;
}

void ConstantPoolClassReference::Prepare( const ConstantPool *pPool )
{
  m_Prepared = true;
  m_pName = pPool->GetConstant( m_Value )->AsString();
}

bool ConstantPoolClassReference::IsPrepared() const
{
  return m_Prepared;
}

boost::intrusive_ptr<JavaString> ConstantPoolClassReference::GetClassName() const
{
#ifdef _DEBUG
  if ( nullptr == m_pName )
  {
    __asm int 3;
  }
#endif // _DEBUG

  return m_pName;
}