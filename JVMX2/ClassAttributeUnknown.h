
#ifndef __CLASSATTRIBUTEUNKOWN_H__
#define __CLASSATTRIBUTEUNKOWN_H__

#include "JavaAttributeUnknown.h"
#include "JavaCodeAttribute.h"
#include "CodeAttributeUnknown.h"

typedef CodeAttributeUnknown ClassAttributeUnknown;

// class ClassAttributeUnknown : public JavaCodeAttribute, public JavaAttributeUnknown
// {
// private:
//   ClassAttributeUnknown( const JavaString &name, uint32_t length, const uint8_t *pBuffer );
//
// public:
//   ClassAttributeUnknown( const ClassAttributeUnknown &other );
//   ClassAttributeUnknown( ClassAttributeUnknown &&other );
//
//   bool Equals( const JavaCodeAttribute &other ) const JVMX_NOEXCEPT JVMX_OVERRIDE;
//   bool Equals( const ClassAttributeUnknown &other ) const JVMX_NOEXCEPT;
//
//   virtual ~ClassAttributeUnknown() JVMX_NOEXCEPT;
//
//   static ClassAttributeUnknown FromBinary( JavaString name, uint32_t length, const uint8_t *pBuffer );
//
//   virtual DataBuffer ToBinary( const ConstantPool & ) const JVMX_OVERRIDE;
//
//  protected:
//    virtual void swap( ClassAttributeUnknown &left, ClassAttributeUnknown &right ) JVMX_NOEXCEPT;
// };

#endif // __CLASSATTRIBUTEUNKOWN_H__
