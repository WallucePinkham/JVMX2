
#ifndef __CONSTANTPOOLINTERFACEMETHODREFERENCE_H__
#define __CONSTANTPOOLINTERFACEMETHODREFERENCE_H__

#include "ConstantPoolMethodReference.h"

class ConstantPoolInterfaceMethodReference : public ConstantPoolMethodReference
{
public:
  ConstantPoolInterfaceMethodReference( std::shared_ptr<ConstantPoolClassReference> pClassRef, ConstantPoolIndex nameAndTypeRefIndex ) : ConstantPoolMethodReference( pClassRef, nameAndTypeRefIndex ) {}
  ConstantPoolInterfaceMethodReference( const ConstantPoolInterfaceMethodReference &other );

  virtual ~ConstantPoolInterfaceMethodReference() JVMX_NOEXCEPT{};
};

inline ConstantPoolInterfaceMethodReference::ConstantPoolInterfaceMethodReference( const ConstantPoolInterfaceMethodReference &other ) : ConstantPoolMethodReference( other )
{}

#endif // __CONSTANTPOOLINTERFACEMETHODREFERENCE_H__
