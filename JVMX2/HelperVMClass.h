#ifndef _HELPERVMCLASS__H_
#define _HELPERVMCLASS__H_

#include "GlobalConstants.h"
#include "TypeParser.h"
#include "include/jni.h"

class HelperVMClass
{
public:
  static jobject JNICALL java_lang_VMClass_forName( JNIEnv *pEnv, jobject obj, jstring className, jboolean initialize, jobject classLoader );
  static jobject JNICALL java_lang_VMClass_getName( JNIEnv *pEnv, jobject obj, jclass clazz );
  static jboolean JNICALL java_lang_VMClass_isArray( JNIEnv *pEnv, jobject obj, jclass clazz );
  static jboolean JNICALL java_lang_VMClass_isPrimitive( JNIEnv *pEnv, jobject obj, jclass clazz );
  static jobject JNICALL java_lang_VMClass_getClassLoader( JNIEnv *pEnv, jobject obj, jclass clazz );
  static jint JNICALL java_lang_VMClass_getModifiers( JNIEnv *pEnv, jobject obj, jclass klass, jboolean ignoreInnerClassesAttrib );

  static jclass JNICALL java_lang_VMClass_getComponentType( JNIEnv *pEnv, jobject obj, jclass klass );

  static jarray JNICALL java_lang_VMClass_getDeclaredFields( JNIEnv *pEnv, jobject obj, jobject classObject, jboolean publicOnly );
  static jarray JNICALL java_lang_VMClass_getDeclaredConstructors( JNIEnv *pEnv, jobject obj, jobject classObject, jboolean publicOnly );
  static jarray JNICALL java_lang_VMClass_getInterfaces( JNIEnv *pEnv, jobject obj, jobject classObject );
  static jobject JNICALL java_lang_VMClass_getSuperclass( JNIEnv *pEnv, jobject obj, jobject classObject );

private:
  static boost::intrusive_ptr<JavaString> GetClassName( boost::intrusive_ptr<ObjectReference> pObject );
  static size_t CountClassConstructors( std::shared_ptr<JavaClass> pClassFile );

  static boost::intrusive_ptr<ObjectReference> InitialiseNewConstructorObject( boost::intrusive_ptr<ObjectReference> pVMConstructor, IVirtualMachineState * pVirtualMachineState, std::shared_ptr<JavaClass> pConstructorClass, std::shared_ptr<MethodInfo> pConstructorToExecute );
  static boost::intrusive_ptr<ObjectReference> InitialiseNewVMConstructor( size_t i, boost::intrusive_ptr<ObjectReference> pDeclaringClass, IVirtualMachineState * pVirtualMachineState, std::shared_ptr<JavaClass> pVMConstructor, std::shared_ptr<MethodInfo> pConstructorToExecute );

  static boost::intrusive_ptr<ObjectReference> InitialiseNewFieldObject( boost::intrusive_ptr<ObjectReference> pVMField, IVirtualMachineState * pVirtualMachineState, std::shared_ptr<JavaClass> pFieldClass, std::shared_ptr<MethodInfo> pConstructorToExecute );
  static boost::intrusive_ptr<ObjectReference> InitialiseNewVMField( size_t i, boost::intrusive_ptr<ObjectReference> pDeclaringClass, const JavaString &fieldName, IVirtualMachineState * pVirtualMachineState, std::shared_ptr<JavaClass> pVMField, std::shared_ptr<MethodInfo> pConstructorToExecute );
};

#endif // _HELPERVMCLASS__H_
