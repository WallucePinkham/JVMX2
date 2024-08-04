
#ifndef __CONSTANTPOOLMETHODREFERENCE_H__
#define __CONSTANTPOOLMETHODREFERENCE_H__

#include "ConstantPoolDualReference.h"

class ConstantPoolMethodReference : public ConstantPoolDualReference
{
public:
  ConstantPoolMethodReference( std::shared_ptr<ConstantPoolClassReference> pClassRef, ConstantPoolIndex nameAndTypeRefIndex ) : ConstantPoolDualReference( pClassRef, nameAndTypeRefIndex ) {}
  ConstantPoolMethodReference( const ConstantPoolMethodReference &other );

  virtual ~ConstantPoolMethodReference() JVMX_NOEXCEPT{};
};

inline ConstantPoolMethodReference::ConstantPoolMethodReference( const ConstantPoolMethodReference &other ) : ConstantPoolDualReference( other )
{}

#endif // __CONSTANTPOOLMETHODREFERENCE_H__
