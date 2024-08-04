
#ifndef __CONSTANTPOOLFIELDREFERENCE_H__
#define __CONSTANTPOOLFIELDREFERENCE_H__

#include "ConstantPoolDualReference.h"

class ConstantPoolFieldReference : public ConstantPoolDualReference
{
public:
  ConstantPoolFieldReference( std::shared_ptr<ConstantPoolClassReference> pClassRef, ConstantPoolIndex nameAndTypeRefIndex ) : ConstantPoolDualReference( pClassRef, nameAndTypeRefIndex ) {}
  ConstantPoolFieldReference( const ConstantPoolFieldReference &other );

  virtual ~ConstantPoolFieldReference() JVMX_NOEXCEPT{};
};

inline ConstantPoolFieldReference::ConstantPoolFieldReference( const ConstantPoolFieldReference &other ) : ConstantPoolDualReference( other )
{}

#endif // __CONSTANTPOOLFIELDREFERENCE_H__
