
#include "ConstantPoolNameAndTypeDescriptor.h"

ConstantPoolNameAndTypeDescriptor::ConstantPoolNameAndTypeDescriptor( ConstantPoolIndex nameIndex, ConstantPoolIndex descriptorIndex ) JVMX_NOEXCEPT : m_NameIndex( nameIndex ), m_DescriptorIndex( descriptorIndex )
{
}

ConstantPoolNameAndTypeDescriptor::ConstantPoolNameAndTypeDescriptor( const ConstantPoolNameAndTypeDescriptor &other ) JVMX_NOEXCEPT : m_NameIndex( other.m_NameIndex ), m_DescriptorIndex( other.m_DescriptorIndex )
{
}

ConstantPoolNameAndTypeDescriptor::~ConstantPoolNameAndTypeDescriptor() JVMX_NOEXCEPT
{
}

ConstantPoolIndex ConstantPoolNameAndTypeDescriptor::GetNameIndex() const JVMX_NOEXCEPT
{
  return m_NameIndex;
}

ConstantPoolIndex ConstantPoolNameAndTypeDescriptor::GetTypeDescriptorIndex() const JVMX_NOEXCEPT
{
  return m_DescriptorIndex;
}
