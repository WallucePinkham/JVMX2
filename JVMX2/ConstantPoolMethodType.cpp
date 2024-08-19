

#include "ConstantPoolMethodType.h"

ConstantPoolMethodType::ConstantPoolMethodType(ConstantPoolIndex descriptorIndex) JVMX_NOEXCEPT
  : m_DescriptorIndex(descriptorIndex)
{
}

ConstantPoolMethodType::ConstantPoolMethodType(const ConstantPoolMethodType& other) JVMX_NOEXCEPT
  : m_DescriptorIndex(other.m_DescriptorIndex)
{
}

ConstantPoolMethodType::~ConstantPoolMethodType() JVMX_NOEXCEPT
{
}

ConstantPoolIndex ConstantPoolMethodType::GetDescriptorIndex() const JVMX_NOEXCEPT
{
  return m_DescriptorIndex;
}
