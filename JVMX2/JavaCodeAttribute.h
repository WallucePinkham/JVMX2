
#ifndef __JAVACODEATTRIBUTE_H__
#define __JAVACODEATTRIBUTE_H__

#include "GlobalConstants.h"
#include "DataBuffer.h"
#include "JavaString.h"

class CodeAttributeUnknown; // Forward declaration;
class AttributeConstantValue; // Forward declaration;
class ClassAttributeCode; // Forward declaration;
class ConstantPool; // Forward declaration;


enum e_JavaAttributeTypes : uint8_t
{
  e_JavaAttributeTypeInvalid
  , e_JavaAttributeTypeUnknown
  , e_JavaAttributeTypeStackMapTable
  , e_JavaAttributeTypeExceptions
  , e_JavaAttributeTypeLineNumberTable
  , e_JavaAttributeTypeLocalVariableTable
  , e_JavaAttributeTypeLocalVariableTypeTable
  , e_JavaAttributeTypeDeprecated
  , e_JavaAttributeTypeRuntimeVisibleAnnotations
  , e_JavaAttributeTypeRuntimeInvisibleAnnotations
  , e_JavaAttributeTypeRuntimeVisibleParameterAnnotations
  , e_JavaAttributeTypeRuntimeInvisibleParameterAnnotations
  , e_JavaAttributeTypeAnnotationDefault
  , e_JavaAttributeTypeConstantValue
  , e_JavaAttributeTypeSynthetic
  , e_JavaAttributeTypeSignature
  , e_JavaAttributeTypeCode
  , e_JavaAttributeTypeInnerClasses
  , e_JavaAttributeTypeEnclosingMethod
  , e_JavaAttributeTypeSourceFile
  , e_JavaAttributeTypeSourceDebugExtension
};

const JavaString c_AttributeNameStackMapTable = JavaString::FromCString( JVMX_T( "StackMapTable" ) );
const JavaString c_AttributeNameAnnotationDefault = JavaString::FromCString( JVMX_T( "AnnotationDefault" ) );
const JavaString c_AttributeNameDeprecated = JavaString::FromCString( JVMX_T( "Deprecated" ) );
const JavaString c_AttributeNameExceptions = JavaString::FromCString( JVMX_T( "Exceptions" ) );
const JavaString c_AttributeNameLineNumberTable = JavaString::FromCString( JVMX_T( "LineNumberTable" ) );
const JavaString c_AttributeNameLocalVariableTable = JavaString::FromCString( JVMX_T( "LocalVariableTable" ) );
const JavaString c_AttributeNameLocalVariableTypeTable = JavaString::FromCString( JVMX_T( "LocalVariableTypeTable" ) );
const JavaString c_AttributeNameRuntimeInvisibleAnnotations = JavaString::FromCString( JVMX_T( "RuntimeInvisibleAnnotations" ) );
const JavaString c_AttributeNameRuntimeInvisibleParameterAnnotations = JavaString::FromCString( JVMX_T( "RuntimeInvisibleParameterAnnotations" ) );
const JavaString c_AttributeNameRuntimeVisibleAnnotations = JavaString::FromCString( JVMX_T( "RuntimeVisibleAnnotations" ) );
const JavaString c_AttributeNameRuntimeVisibleParameterAnnotations = JavaString::FromCString( JVMX_T( "RuntimeVisibleParameterAnnotations" ) );
const JavaString c_AttributeNameSignature = JavaString::FromCString( JVMX_T( "Signature" ) );
const JavaString c_AttributeNameConstantValue = JavaString::FromCString( JVMX_T( "ConstantValue" ) );
const JavaString c_AttributeNameCode = JavaString::FromCString( JVMX_T( "Code" ) );
const JavaString c_AttributeNameDebugExtension = JavaString::FromCString( JVMX_T( "SourceDebugExtension" ) );
const JavaString c_AttributeNameInnerClasses = JavaString::FromCString( JVMX_T( "InnerClasses" ) );
const JavaString c_AttributeNameBootstrapMethods = JavaString::FromCString( JVMX_T( "BootstrapMethods" ) );
const JavaString c_AttributeNameEnclosingMethod = JavaString::FromCString( JVMX_T( "EnclosingMethod" ) );
const JavaString c_AttributeNameSourceFile = JavaString::FromCString( JVMX_T( "SourceFile" ) );
const JavaString c_AttributeNameSynthetic = JavaString::FromCString( JVMX_T( "Synthetic" ) );
const JavaString c_AttributeNameUnknown = JavaString::FromCString( JVMX_T( "Unknown" ) );

class JavaCodeAttribute JVMX_ABSTRACT
{
protected:
  JavaCodeAttribute( const JavaString &name, e_JavaAttributeTypes type ) JVMX_NOEXCEPT;

public:
  virtual ~JavaCodeAttribute() JVMX_NOEXCEPT;

  virtual JavaString GetName() const;
  virtual e_JavaAttributeTypes GetType() const;

  virtual DataBuffer ToBinary( const ConstantPool & ) const JVMX_PURE;

  virtual bool Equals( const JavaCodeAttribute &other ) const JVMX_PURE;

  virtual bool operator==(const JavaCodeAttribute &other) const;

  virtual CodeAttributeUnknown ToUnknown() const;
  virtual AttributeConstantValue ToConstantValue() const;
  virtual ClassAttributeCode ToCode() const;

protected:
  JavaCodeAttribute( JavaCodeAttribute &&other ); // Subclasses need to call this
  JavaCodeAttribute( const JavaCodeAttribute &other ); // Subclasses need to call this

  static void swap( JavaCodeAttribute &left, JavaCodeAttribute &right ) JVMX_NOEXCEPT;

private:
  JavaCodeAttribute &operator=(JavaCodeAttribute other) JVMX_FN_DELETE;

protected:
  e_JavaAttributeTypes m_Type;
  JavaString m_Name;
};

#endif // __JAVACODEATTRIBUTE_H__
