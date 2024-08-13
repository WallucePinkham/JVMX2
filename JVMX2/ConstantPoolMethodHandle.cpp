
#include "ConstantPoolMethodHandle.h"

ConstantPoolMethodHandle::ConstantPoolMethodHandle(uint8_t referenceKind,
  ConstantPoolIndex referenceIndex) JVMX_NOEXCEPT
  : m_ReferenceKind(referenceKind), m_ReferenceIndex(referenceIndex)
{
}

ConstantPoolMethodHandle::ConstantPoolMethodHandle(const ConstantPoolMethodHandle& other) JVMX_NOEXCEPT
  : m_ReferenceKind(other.m_ReferenceKind), m_ReferenceIndex(other.m_ReferenceIndex)
{
}

ConstantPoolMethodHandle::~ConstantPoolMethodHandle() JVMX_NOEXCEPT
{
}

uint8_t ConstantPoolMethodHandle::GetReferenceKind() const JVMX_NOEXCEPT
{
  return m_ReferenceKind;
}

ConstantPoolIndex ConstantPoolMethodHandle::GetReferenceIndex() const JVMX_NOEXCEPT
{
  return m_ReferenceIndex;
}