
#ifndef _CLASSFILEFACTORY__H_
#define _CLASSFILEFACTORY__H_

#include "JavaClass.h"
#include "ObjectReference.h"

class ClassFactory JVMX_ABSTRACT
{
public:
  static std::shared_ptr<JavaClass> CreateClassFile( uint16_t minorVersion, uint16_t majorVersion, std::shared_ptr<ConstantPool> pConstantPool, uint16_t accessFlags, ConstantPoolIndex thisClassIndex, ConstantPoolIndex superIndex, InterfaceInfoList interfaces, FieldInfoList fields, MethodInfoList methods, CodeAttributeList attributes )
  {
    return std::shared_ptr<JavaClass>( new JavaClass( minorVersion, majorVersion, pConstantPool, accessFlags, thisClassIndex, superIndex, std::move( interfaces ), std::move( fields ), std::move( methods ), std::move( attributes ), nullptr ) );
  }
};

#endif // _CLASSFILEFACTORY__H_


