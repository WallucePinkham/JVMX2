
#ifndef __CONSTANTPOOL_METHODTYPE_H__
#define __CONSTANTPOOL_METHODTYPE_H__

#include "GlobalConstants.h"
#include "IConstantPoolEntryValue.h"

class ConstantPoolMethodType : public IConstantPoolEntryValue
{
public:
  ConstantPoolMethodType(ConstantPoolIndex descriptorIndex) JVMX_NOEXCEPT;
  ConstantPoolMethodType(const ConstantPoolMethodType& other) JVMX_NOEXCEPT;

  virtual ~ConstantPoolMethodType() JVMX_NOEXCEPT;

  virtual ConstantPoolIndex GetDescriptorIndex() const JVMX_NOEXCEPT;

private:
  ConstantPoolIndex m_DescriptorIndex;
};

#endif // __CONSTANTPOOL_METHODTYPE_H__
