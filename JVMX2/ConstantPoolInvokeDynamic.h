
#ifndef __CONSTANTPOOL_INVOKEDYNAMICINFO_H__
#define __CONSTANTPOOL_INVOKEDYNAMICINFO_H__

#include "GlobalConstants.h"
#include "IConstantPoolEntryValue.h"

class ConstantPoolInvokeDynamic : public IConstantPoolEntryValue
{
public:
  ConstantPoolInvokeDynamic(ConstantPoolIndex bootstrapMethodAttrIndex, ConstantPoolIndex nameAndTypeIndex) JVMX_NOEXCEPT;
  ConstantPoolInvokeDynamic(const ConstantPoolInvokeDynamic& other) JVMX_NOEXCEPT;

  virtual ~ConstantPoolInvokeDynamic() JVMX_NOEXCEPT;

  virtual ConstantPoolIndex GetBootstrapMethodAttrIndex() const JVMX_NOEXCEPT;
  virtual ConstantPoolIndex GetNameAndTypeIndex() const JVMX_NOEXCEPT;

private:
  ConstantPoolIndex m_BootstrapMethodAttrIndex;
  ConstantPoolIndex m_NameAndTypeIndex;
};

#endif // __CONSTANTPOOL_INVOKEDYNAMICINFO_H__

