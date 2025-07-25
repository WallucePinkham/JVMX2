#ifndef __CONSTANTPOOL_METHODHANDLE_H__
#define __CONSTANTPOOL_METHODHANDLE_H__

#include "GlobalConstants.h"
#include "IConstantPoolEntryValue.h"

class ConstantPoolMethodHandle : public IConstantPoolEntryValue
{
public:
  ConstantPoolMethodHandle(uint8_t referenceKind, ConstantPoolIndex referenceIndex) JVMX_NOEXCEPT;
  ConstantPoolMethodHandle(const ConstantPoolMethodHandle& other) JVMX_NOEXCEPT;

  virtual ~ConstantPoolMethodHandle() JVMX_NOEXCEPT;

  virtual uint8_t GetReferenceKind() const JVMX_NOEXCEPT;
  virtual ConstantPoolIndex GetReferenceIndex() const JVMX_NOEXCEPT;

private:
  uint8_t m_ReferenceKind;
  ConstantPoolIndex m_ReferenceIndex;
};

#endif // __CONSTANTPOOL_METHODHANDLE_H__

