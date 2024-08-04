#include "InvalidStateException.h"

#include "ConstantPool.h"

#include "ConstantPoolDualReference.h"

ConstantPoolDualReference::ConstantPoolDualReference( std::shared_ptr<ConstantPoolClassReference> pClassRef, ConstantPoolIndex nameAndTypeRefIndex )
  : m_pClassRef( pClassRef )
  , m_NameAndTypeRefIndex( nameAndTypeRefIndex )
  , m_Prepared( false )
{}

ConstantPoolDualReference::ConstantPoolDualReference( const ConstantPoolDualReference &other )
  : m_pClassRef( other.m_pClassRef )
  , m_NameAndTypeRefIndex( other.m_NameAndTypeRefIndex )
  , m_Prepared( other.m_Prepared )
{}

ConstantPoolDualReference::~ConstantPoolDualReference() JVMX_NOEXCEPT
{}

bool ConstantPoolDualReference::operator==( const ConstantPoolDualReference &other ) const JVMX_NOEXCEPT
{
  return (*m_pClassRef == *other.m_pClassRef) && (m_NameAndTypeRefIndex == other.m_NameAndTypeRefIndex);
}

std::shared_ptr<ConstantPoolClassReference> ConstantPoolDualReference::GetClassReference() const JVMX_NOEXCEPT
{
  return m_pClassRef;
}

ConstantPoolIndex ConstantPoolDualReference::GetNameAndTypeReferenceIndex() const JVMX_NOEXCEPT
{
  return m_NameAndTypeRefIndex;
}

const ConstantPoolDualReference & ConstantPoolDualReference::operator=( const ConstantPoolDualReference &other )
{
  m_pClassRef = other.m_pClassRef;
  m_NameAndTypeRefIndex = other.m_NameAndTypeRefIndex;

  m_Prepared = other.m_Prepared;
  m_ClassName = other.m_ClassName;
  m_Name = other.m_Name;
  m_Type = other.m_Type;

  return *this;
}

boost::intrusive_ptr<JavaString> ConstantPoolDualReference::GetClassName() const
{
  if ( !m_Prepared )
  {
    throw InvalidStateException( __FUNCTION__ " - This class has not been prepared." );
  }

  return m_ClassName;
}

boost::intrusive_ptr<JavaString> ConstantPoolDualReference::GetName() const
{
  if ( !m_Prepared )
  {
    throw InvalidStateException( __FUNCTION__ " - This class has not been prepared." );
  }

  return m_Name;
}

boost::intrusive_ptr<JavaString> ConstantPoolDualReference::GetType() const
{
  if ( !m_Prepared )
  {
    throw InvalidStateException( __FUNCTION__ " - This class has not been prepared." );
  }

  return m_Type;
}

void ConstantPoolDualReference::Prepare( const ConstantPool *pPool )
{
  std::shared_ptr<ConstantPoolClassReference> pClassRef = pPool->GetConstant( m_pClassRef->ToConstantPoolIndex() )->AsClassReferencePointer();
  m_ClassName = pPool->GetConstant( pClassRef->ToConstantPoolIndex() )->AsString();

  std::shared_ptr<ConstantPoolNameAndTypeDescriptor> pNameAndType = pPool->GetConstant( m_NameAndTypeRefIndex )->AsNameAndTypeDescriptorPointer();
  m_Name = pPool->GetConstant( pNameAndType->GetNameIndex() )->AsString();
  m_Type = pPool->GetConstant( pNameAndType->GetTypeDescriptorIndex() )->AsString();

  m_Prepared = true;
}