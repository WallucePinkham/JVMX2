#include <memory>

#include "JavaString.h"

#include "CodeAttributeStackMapTable.h"
#include "CodeAttributeAnnotationDefault.h"
#include "CodeAttributeUnknown.h"
#include "CodeAttributeDeprecated.h"
#include "CodeAttributeExceptions.h"
#include "CodeAttributeLineNumberTable.h"
#include "CodeAttributeLocalVariableTable.h"
#include "CodeAttributeLocalVariableTypeTable.h"
#include "CodeAttributeRuntimeInvisibleAnnotations.h"
#include "CodeAttributeRuntimeInvisibleParameterAnnotations.h"
#include "CodeAttributeRuntimeVisibleAnnotations.h"
#include "CodeAttributeRuntimeVisibleParameterAnnotations.h"
#include "CodeAttributeSignature.h"

#include "AttributeConstantValue.h"
#include "ClassAttributeCode.h"
#include "ClassAttributeUnknown.h"
#include "ClassAttributeSourceDebugExtension.h"
#include "ClassAttributeInnerClasses.h"
#include "ClassAttributeBootstrapMethods.h"
#include "ClassAttributeEnclosingMethod.h"
#include "ClassAttributeSourceFile.h"
#include "ClassAttributeSynthetic.h"

#include "CodeAttributeFactory.h"

class ConstantPool; // Forward declaration

std::shared_ptr<JavaCodeAttribute> CodeAttributeFactory::CreateFromBinary( const JavaString &name, Stream &byteStream, const ConstantPool &constantPool, size_t byteCount )
{
  if ( name == c_AttributeNameStackMapTable )
  {
    return std::make_shared<CodeAttributeStackMapTable>( CodeAttributeStackMapTable::FromBinary( byteStream ) );
  }

  if ( name == c_AttributeNameAnnotationDefault )
  {
    return std::make_shared<CodeAttributeAnnotationDefault>( CodeAttributeAnnotationDefault::FromBinary( name, byteStream ) );
  }

  if ( name == c_AttributeNameDeprecated )
  {
    return std::make_shared<CodeAttributeDeprecated>( CodeAttributeDeprecated::FromBinary( name ) );
  }

  if ( name == c_AttributeNameExceptions )
  {
    return std::make_shared<CodeAttributeExceptions>( CodeAttributeExceptions::FromBinary( name, byteStream ) );
  }

  if ( name == c_AttributeNameLineNumberTable )
  {
    return std::make_shared<CodeAttributeLineNumberTable>( CodeAttributeLineNumberTable::FromBinary( name, byteStream ) );
  }

  if ( name == c_AttributeNameLocalVariableTable )
  {
    return std::make_shared<CodeAttributeLocalVariableTable>( CodeAttributeLocalVariableTable::FromBinary( name, byteStream, constantPool ) );
  }

  if ( name == c_AttributeNameLocalVariableTypeTable )
  {
    return std::make_shared<CodeAttributeLocalVariableTypeTable>( CodeAttributeLocalVariableTypeTable::FromBinary( name, byteStream ) );
  }

  if ( name == c_AttributeNameRuntimeInvisibleAnnotations )
  {
    return std::make_shared<CodeAttributeRuntimeInvisibleAnnotations>( CodeAttributeRuntimeInvisibleAnnotations::FromBinary( name, byteStream ) );
  }

  if ( name == c_AttributeNameRuntimeInvisibleParameterAnnotations )
  {
    return std::make_shared<CodeAttributeRuntimeInvisibleParameterAnnotations>( CodeAttributeRuntimeInvisibleParameterAnnotations::FromBinary( name, byteStream ) );
  }

  if ( name == c_AttributeNameRuntimeVisibleAnnotations )
  {
    return std::make_shared<CodeAttributeRuntimeVisibleAnnotations>( CodeAttributeRuntimeVisibleAnnotations::FromBinary( name, byteStream ) );
  }

  if ( name == c_AttributeNameRuntimeVisibleParameterAnnotations )
  {
    return std::make_shared<CodeAttributeRuntimeVisibleParameterAnnotations>( CodeAttributeRuntimeVisibleParameterAnnotations::FromBinary( name, byteStream ) );
  }

  if ( name == c_AttributeNameSignature )
  {
    return std::make_shared<CodeAttributeSignature>( CodeAttributeSignature::FromBinary( name, byteStream ) );
  }

  if ( c_AttributeNameConstantValue == name )
  {
    return std::make_shared<AttributeConstantValue>( AttributeConstantValue::FromBinary( name, byteStream ) );
  }

  if ( c_AttributeNameCode == name )
  {
    return std::make_shared<ClassAttributeCode>( ClassAttributeCode::FromBinary( byteStream, *this, constantPool ) );
  }

  if ( c_AttributeNameDebugExtension == name )
  {
    return std::make_shared<ClassAttributeSourceDebugExtension>( ClassAttributeSourceDebugExtension::FromBinary( name, byteStream ) );
  }

  if ( c_AttributeNameInnerClasses == name )
  {
    return std::make_shared<ClassAttributeInnerClasses>( ClassAttributeInnerClasses::FromBinary( name, byteStream ) );
  }

  if ( c_AttributeNameBootstrapMethods == name )
  {
    return std::make_shared<ClassAttributeBootstrapMethods>( ClassAttributeBootstrapMethods::FromBinary( name, byteStream ) );
  }

  if ( c_AttributeNameEnclosingMethod == name )
  {
    return std::make_shared<ClassAttributeEnclosingMethod>( ClassAttributeEnclosingMethod::FromBinary( name, byteStream ) );
  }

  if ( c_AttributeNameSourceFile == name )
  {
    return std::make_shared<ClassAttributeSourceFile>( ClassAttributeSourceFile::FromBinary( name, byteStream ) );
  }

  if ( c_AttributeNameSynthetic == name )
  {
    return std::make_shared<ClassAttributeSynthetic>( ClassAttributeSynthetic::FromBinary( name ) );
  }

  return std::make_shared<CodeAttributeUnknown>( CodeAttributeUnknown::FromBinary( name, byteStream, byteCount ) );
}