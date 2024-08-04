
#ifndef __CONSTANTPOOLNAMEANDTYPEDESCRIPTOR_H__
#define __CONSTANTPOOLNAMEANDTYPEDESCRIPTOR_H__

#include "GlobalConstants.h"
#include "IConstantPoolEntryValue.h"

class ConstantPoolNameAndTypeDescriptor : public IConstantPoolEntryValue
{
public:
  ConstantPoolNameAndTypeDescriptor( ConstantPoolIndex nameIndex, ConstantPoolIndex descriptorIndex ) JVMX_NOEXCEPT;
  ConstantPoolNameAndTypeDescriptor( const ConstantPoolNameAndTypeDescriptor &other ) JVMX_NOEXCEPT;

  virtual ~ConstantPoolNameAndTypeDescriptor() JVMX_NOEXCEPT;

  virtual ConstantPoolIndex GetNameIndex() const JVMX_NOEXCEPT;
  virtual ConstantPoolIndex GetTypeDescriptorIndex() const JVMX_NOEXCEPT;

private:
  ConstantPoolIndex m_NameIndex;
  ConstantPoolIndex m_DescriptorIndex;
};

#endif // __CONSTANTPOOLNAMEANDTYPEDESCRIPTOR_H__
