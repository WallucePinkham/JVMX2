#include "ConstantPoolInvokeDynamic.h"

ConstantPoolInvokeDynamic::ConstantPoolInvokeDynamic(ConstantPoolIndex bootstrapMethodAttrIndex, 
                                                             ConstantPoolIndex nameAndTypeIndex) JVMX_NOEXCEPT
  : m_BootstrapMethodAttrIndex(bootstrapMethodAttrIndex), m_NameAndTypeIndex(nameAndTypeIndex)
{
}

ConstantPoolInvokeDynamic::ConstantPoolInvokeDynamic(const ConstantPoolInvokeDynamic& other) JVMX_NOEXCEPT 
  : m_BootstrapMethodAttrIndex(other.m_BootstrapMethodAttrIndex), m_NameAndTypeIndex(other.m_NameAndTypeIndex)
{
}

ConstantPoolInvokeDynamic::~ConstantPoolInvokeDynamic() JVMX_NOEXCEPT
{
}

ConstantPoolIndex ConstantPoolInvokeDynamic::GetBootstrapMethodAttrIndex() const JVMX_NOEXCEPT
{
  return m_BootstrapMethodAttrIndex;
}

ConstantPoolIndex ConstantPoolInvokeDynamic::GetNameAndTypeIndex() const JVMX_NOEXCEPT
{
  return m_NameAndTypeIndex;
}