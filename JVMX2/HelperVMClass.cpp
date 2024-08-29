#include <boost\intrusive_ptr.hpp>

#include "GlobalConstants.h"

#include "JavaExceptionConstants.h"

#include "FileDoesNotExistException.h"
#include "InvalidStateException.h"
#include "NotImplementedException.h"


#include "ILogger.h"
#include "IVirtualMachineState.h"
#include "IClassLibrary.h"
#include "JavaNativeInterface.h"

#include "IExecutionEngine.h"

#include "TypeParser.h"


#include "VirtualMachine.h"
#include "GlobalCatalog.h"

#include "ObjectReference.h"
#include "HelperClasses.h"
#include "HelperTypes.h"

#include "HelperVMClass.h"

//#define JVMX_LOG_VERBOSE

extern const JavaString c_InstanceInitialisationMethodName;
extern const JavaString c_SyntheticField_ClassName;

//const char *c_JavaLangReflectConstructorClassName = "java/lang/reflect/Constructor";
const JavaString c_VMConstructorClassName = JavaString::FromCString( u"java/lang/reflect/VMConstructor" );
const JavaString c_JavaLangReflectConstructorClassName = JavaString::FromCString( u"java/lang/reflect/Constructor" );
const JavaString c_VMConstructor_ConstructorType = JavaString::FromCString( u"(Ljava/lang/Class;I)V" );
const JavaString c_JavaLangReflectConstructor_ConstructorType = JavaString::FromCString( u"(Ljava/lang/reflect/VMConstructor;)V" );
const JavaString c_VMFieldClassName = JavaString::FromCString( u"java/lang/reflect/VMField" );
const JavaString c_JavaLangReflectFieldClassName = JavaString::FromCString( u"java/lang/reflect/Field" );
const JavaString c_JavaLangReflectField_ConstructorType = JavaString::FromCString( u"(Ljava/lang/reflect/VMField;)V" );
const JavaString c_VMField_ConstructorType = JavaString::FromCString( u"(Ljava/lang/Class;Ljava/lang/String;I)V" );

jobject JNICALL HelperVMClass::java_lang_VMClass_forName( JNIEnv *pEnv, jobject obj, jstring className, jboolean initialize, jobject classLoader )
{
  boost::intrusive_ptr<ObjectReference> pClassName = JNIEnvInternal::ConvertJObjectToObjectPointer( className );

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMClass_forName(%s, %d, %p)\n", reinterpret_cast<const char *>( pClassName->ToString().ToUtf8String().c_str() ), initialize ? 1 : 0, classLoader );
#endif // _DEBUG

  JavaString pTemp = HelperTypes::ExtractValueFromStringObject(pClassName);
  boost::intrusive_ptr<JavaString> pClassNameAsJavaString = new JavaString(pTemp);

  //boost::intrusive_ptr<ObjectReference> pValueArray = boost::dynamic_pointer_cast<ObjectReference>( ->GetContainedObject()->GetFieldByName( JavaString::FromCString( "value" ) ) );
  //boost::intrusive_ptr<JavaString> pClassNameAsJavaString = new JavaString( JavaString::FromArray( *( pValueArray->GetContainedArray() ) ) );

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  // I don't like the TrimRight here, I would like to know where the spaces come from :(
  *pClassNameAsJavaString = pClassNameAsJavaString->ReplaceAll( u'.', u'/' ).TrimRight();
  try
  {
    pVirtualMachineState->InitialiseClass( *pClassNameAsJavaString );
  }
  catch ( const FileDoesNotExistException & )
  {
    //std::shared_ptr<IExecutionEngine> pEngine = GlobalCatalog::GetInstance().Get( "ExecutionEngine" );
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaJavaClassNotFoundException );
    return nullptr;
  }

  boost::intrusive_ptr<ObjectReference> pResult = pVirtualMachineState->CreateJavaLangClassFromClassName( pClassNameAsJavaString );

  return JNIEnvInternal::ConvertObjectPointerToJObject( pVirtualMachineState, pResult );
}

jobject JNICALL HelperVMClass::java_lang_VMClass_getName( JNIEnv *pEnv, jobject obj, jclass clazz )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMClass_getName()\n" );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  boost::intrusive_ptr<ObjectReference> pObject = JNIEnvInternal::ConvertJObjectToObjectPointer( clazz );
  boost::intrusive_ptr<JavaString> pClassName = boost::dynamic_pointer_cast<JavaString>( pObject->GetContainedObject()->GetJVMXFieldByName( c_SyntheticField_ClassName ) );

  boost::intrusive_ptr<ObjectReference> pName = pVirtualMachineState->CreateStringObject( pClassName->ReplaceAll( u'/', u'.' ) );

  return JNIEnvInternal::ConvertObjectPointerToJObject( pVirtualMachineState, pName );
}

jboolean JNICALL HelperVMClass::java_lang_VMClass_isArray( JNIEnv *pEnv, jobject obj, jclass clazz )
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMClass_isArray()\n" );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  boost::intrusive_ptr<ObjectReference> pObject = JNIEnvInternal::ConvertJObjectToObjectPointer( clazz );

  boost::intrusive_ptr<JavaString> pClassName = boost::dynamic_pointer_cast<JavaString>( pObject->GetContainedObject()->GetJVMXFieldByName( c_SyntheticField_ClassName ) );

  if ( nullptr == pClassName )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected class name to be valid." );
  }

  if ( '[' == pClassName->At( 0 ) )
  {
    return static_cast<jboolean>( JNI_TRUE );
  }

  return static_cast<jboolean>( JNI_FALSE );
}

jboolean JNICALL HelperVMClass::java_lang_VMClass_isPrimitive( JNIEnv *pEnv, jobject obj, jclass clazz )
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMClass_isPrimitive(%p)\n", clazz );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  boost::intrusive_ptr<ObjectReference> pObject = JNIEnvInternal::ConvertJObjectToObjectPointer( clazz );
  boost::intrusive_ptr<JavaString> pClassName = GetJavaLangClassName( pObject );

  if ( *pClassName == JavaString::FromCString( "java/lang/Integer" ) )
  {
    return true;
  }

  if ( *pClassName == JavaString::FromCString( "java/lang/Boolean" ) )
  {
    return true;
  }

  if ( *pClassName == JavaString::FromCString( "java/lang/Character" ) )
  {
    return true;
  }

  if ( *pClassName == JavaString::FromCString( "java/lang/Void" ) )
  {
    return true;
  }

  if ( *pClassName == JavaString::FromCString( "java/lang/Byte" ) )
  {
    return true;
  }

  if ( *pClassName == JavaString::FromCString( "java/lang/Short" ) )
  {
    return true;
  }

  if ( *pClassName == JavaString::FromCString( "java/lang/Long" ) )
  {
    return true;
  }

  if ( *pClassName == JavaString::FromCString( "java/lang/Float" ) )
  {
    return true;
  }

  if ( *pClassName == JavaString::FromCString( "java/lang/Double" ) )
  {
    return true;
  }

  return false;
}

jobject JNICALL HelperVMClass::java_lang_VMClass_getClassLoader( JNIEnv *pEnv, jobject obj, jclass clazz )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMClass_getClassLoader" );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  boost::intrusive_ptr<ObjectReference> pObject = JNIEnvInternal::ConvertJObjectToObjectPointer( clazz );
  boost::intrusive_ptr<ObjectReference> pClassLoader = pVirtualMachineState->GetClassLoaderForClassObject( pObject );

  if ( nullptr == pClassLoader )
  {
    return JNIEnvInternal::ConvertNullPointerToJObject();
  }

  return JNIEnvInternal::ConvertObjectPointerToJObject( pVirtualMachineState, pClassLoader );
}

jint JNICALL HelperVMClass::java_lang_VMClass_getModifiers( JNIEnv *pEnv, jobject obj, jclass klass, jboolean ignoreInnerClassesAttrib )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMClass_getModifiers" );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  boost::intrusive_ptr<ObjectReference> pObject = JNIEnvInternal::ConvertJObjectToObjectPointer( klass );

  boost::intrusive_ptr<JavaString> pClazzName = boost::dynamic_pointer_cast<JavaString>( pObject->GetContainedObject()->GetJVMXFieldByName( c_SyntheticField_ClassName ) );
  std::shared_ptr<JavaClass> pClazzValue = pVirtualMachineState->InitialiseClass( *pClazzName );

  return pClazzValue->GetModifiers();
}

jclass JNICALL HelperVMClass::java_lang_VMClass_getComponentType( JNIEnv *pEnv, jobject obj, jclass klass )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMClass_getComponentType(%p)\n", klass );
#endif // _DEBUG

  boost::intrusive_ptr<ObjectReference> pObject = JNIEnvInternal::ConvertJObjectToObjectPointer( klass );
  boost::intrusive_ptr<JavaString> pClassName = GetJavaLangClassName( pObject );

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  if ( pClassName->At( 0 ) != c_JavaTypeSpecifierArray )
  {
    return static_cast<jclass>( JNIEnvInternal::ConvertNullPointerToJObject() );
  }

  boost::intrusive_ptr<JavaString> pComponentName = new JavaString( pClassName->SubString( 1 ) );
  return static_cast<jclass>( JNIEnvInternal::ConvertObjectPointerToJObject( pVirtualMachineState, pVirtualMachineState->CreateJavaLangClassFromClassName( pComponentName ) ) );
}

jarray JNICALL HelperVMClass::java_lang_VMClass_getDeclaredFields( JNIEnv *pEnv, jobject obj, jobject classObject, jboolean publicOnly )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMClass_getDeclaredFields()\n" );
#endif // _DEBUG

  boost::intrusive_ptr<ObjectReference> pObject = JNIEnvInternal::ConvertJObjectToObjectPointer( classObject );
  boost::intrusive_ptr<JavaString> pClassName = GetJavaLangClassName( pObject );

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  std::shared_ptr<JavaClass> pTargetClass = pVirtualMachineState->InitialiseClass( *pClassName );
  const size_t c_NumberOfFieldsFound = pTargetClass->GetLocalFieldCount( publicOnly ? e_PublicOnly::Yes : e_PublicOnly::No );

#ifdef _DEBUG
  const size_t c_LocalNonStaticFieldsFound = pTargetClass->GetLocalNonStaticFieldCount( e_PublicOnly::Yes );
#endif // _DEBUG

  std::shared_ptr<JavaClass> pVMFieldClass = pVirtualMachineState->InitialiseClass( c_VMFieldClassName );
  /*if ( !pVMFieldClass->IsInitialsed() )
  {
    pVirtualMachineState->InitialiseClass( *pVMFieldClass->GetName() );
  }*/
  std::shared_ptr<MethodInfo> pVMFieldConstructor = pVMFieldClass->GetMethodByNameAndType( c_InstanceInitialisationMethodName, c_VMField_ConstructorType );

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  pLogger->LogDebug( "\tLoading java/lang/reflect/Field\n" );
#endif // _DEBUG

  // Load java/lang/reflect/Constructor
  std::shared_ptr<JavaClass> pJavaLangReflectField = pVirtualMachineState->InitialiseClass( c_JavaLangReflectFieldClassName );
  //if ( !pJavaLangReflectField->IsInitialsed() )
  //{
  //  pVirtualMachineState->InitialiseClass( *pJavaLangReflectField->GetName() );
  //}
  std::shared_ptr<MethodInfo> pFieldConstructor = pJavaLangReflectField->GetMethodByNameAndType( c_InstanceInitialisationMethodName, c_JavaLangReflectField_ConstructorType );

  boost::intrusive_ptr<ObjectReference> pClassObject = pObject;

  boost::intrusive_ptr<ObjectReference> pResults = pVirtualMachineState->CreateArray( e_JavaArrayTypes::Reference, c_NumberOfFieldsFound );

  size_t fieldCounter = 0;
  // It is important that we go through ALL the fields here, even non-public ones, otherwise the slot will be incorrect.
  for ( size_t slotNumber = 0; slotNumber < pTargetClass->GetLocalFieldCount( e_PublicOnly::No ); ++slotNumber )
  {
    std::shared_ptr<FieldInfo> pFieldInfo = pTargetClass->GetFieldByIndex( slotNumber );

    if ( !publicOnly || ( publicOnly && pFieldInfo->IsPublic() ) )
    {
      boost::intrusive_ptr<ObjectReference> pNewVMField = InitialiseNewVMField( slotNumber, pClassObject, * pFieldInfo->GetName(), pVirtualMachineState, pVMFieldClass, pVMFieldConstructor );
      boost::intrusive_ptr<ObjectReference> pNewField = InitialiseNewFieldObject( pNewVMField, pVirtualMachineState, pJavaLangReflectField, pFieldConstructor );

      pResults->GetContainedArray()->SetAt( fieldCounter, pNewField.get() );
      ++fieldCounter;

#ifdef _DEBUG
      JVMX_ASSERT( fieldCounter <= c_NumberOfFieldsFound );
#endif // _DEBUG

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
      pLogger->LogDebug( "\tField construction completed: %s.", pResults->GetContainedArray()->At( fieldCounter - 1 )->ToString().ToUtf8String().c_str() );
#endif // _DEBUG
    }
  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  pVirtualMachineState->LogOperandStack();
  pVirtualMachineState->LogCallStack();

  pLogger->LogDebug( "\tField array construction completed: %s.", pResults->ToString().ToUtf8String().c_str() );
#endif // _DEBUG

  return JNIEnvInternal::ConvertArrayPointerToJArray( pVirtualMachineState, pResults );
}

jarray JNICALL HelperVMClass::java_lang_VMClass_getDeclaredConstructors( JNIEnv *pEnv, jobject obj, jobject classObject, jboolean publicOnly )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMClass_getDeclaredConstructors()\n" );
#endif // _DEBUG

  boost::intrusive_ptr<ObjectReference> pObject = JNIEnvInternal::ConvertJObjectToObjectPointer( classObject );
  boost::intrusive_ptr<JavaString> pClassName = GetJavaLangClassName( pObject );

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  pLogger->LogDebug( "\tGetting declared constructors for class %s\n", pClassName->ToUtf8String().c_str() );
#endif // _DEBUG

  std::shared_ptr<JavaClass> pTargetClass = pVirtualMachineState->InitialiseClass( *pClassName );
  const size_t c_NumberOfConstructorsFound = CountClassConstructors( pTargetClass );

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  pLogger->LogDebug( "\tLoading class VMConstructor\n" );
#endif // _DEBUG

  // Load VMConstructor
  std::shared_ptr<JavaClass> pVMConstructorClass = pVirtualMachineState->InitialiseClass( c_VMConstructorClassName );
  /*if ( !pVMConstructorClass->IsInitialsed() )
  {
    pVirtualMachineState->InitialiseClass( *pVMConstructorClass->GetName() );
  }*/
  std::shared_ptr<MethodInfo> pVMConstructorConstructor = pVMConstructorClass->GetMethodByNameAndType( c_InstanceInitialisationMethodName, c_VMConstructor_ConstructorType );

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  pLogger->LogDebug( "\tLoading java/lang/reflect/Constructor\n" );
#endif // _DEBUG

  // Load java/lang/reflect/Constructor
  std::shared_ptr<JavaClass> pJavaLangReflectConstructor = pVirtualMachineState->InitialiseClass( c_JavaLangReflectConstructorClassName );
  //if ( !pJavaLangReflectConstructor->IsInitialsed() )
  //{
  //  pVirtualMachineState->InitialiseClass( *pJavaLangReflectConstructor->GetName() );
  //}
  std::shared_ptr<MethodInfo> pConstructorConstructor = pJavaLangReflectConstructor->GetMethodByNameAndType( c_InstanceInitialisationMethodName, c_JavaLangReflectConstructor_ConstructorType );

  boost::intrusive_ptr<ObjectReference> pClassObject = pObject;

  boost::intrusive_ptr<ObjectReference> pResults = pVirtualMachineState->CreateArray( e_JavaArrayTypes::Reference, c_NumberOfConstructorsFound );

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  pLogger->LogDebug( "\tPopulating constructor array.\n" );
#endif // _DEBUG

  size_t constructorCounter = 0;
  for ( size_t i = 0; i < pTargetClass->GetMethodCount(); ++i )
  {
    std::shared_ptr<MethodInfo> pMethodInfo = pTargetClass->GetMethodByIndex( i );
    if ( c_InstanceInitialisationMethodName == *pMethodInfo->GetName() )
    {
      boost::intrusive_ptr<ObjectReference> pNewVMConstructor = InitialiseNewVMConstructor( i, pClassObject, pVirtualMachineState, pVMConstructorClass, pVMConstructorConstructor );

      boost::intrusive_ptr<ObjectReference> pNewConstructor = InitialiseNewConstructorObject( pNewVMConstructor, pVirtualMachineState, pJavaLangReflectConstructor, pConstructorConstructor );

      pResults->GetContainedArray()->SetAt( constructorCounter, pNewConstructor.get() );
      ++constructorCounter;

#ifdef _DEBUG
      JVMX_ASSERT( constructorCounter <= c_NumberOfConstructorsFound );
#endif // _DEBUG
    }
  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  pVirtualMachineState->LogOperandStack();
  pVirtualMachineState->LogCallStack();

  pLogger->LogDebug( "\tConstructor array construction completed.\n" );
#endif // _DEBUG

  return JNIEnvInternal::ConvertArrayPointerToJArray( pVirtualMachineState, pResults );
}

jarray JNICALL HelperVMClass::java_lang_VMClass_getInterfaces( JNIEnv *pEnv, jobject obj, jobject classObject )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMClass_getInterfaces()\n" );
#endif // _DEBUG

  boost::intrusive_ptr<ObjectReference> pObject = JNIEnvInternal::ConvertJObjectToObjectPointer( classObject );
  boost::intrusive_ptr<JavaString> pClassName = GetJavaLangClassName( pObject );

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  pLogger->LogDebug( "\tGetting declared interfaces for class %s\n", pClassName->ToUtf8String().c_str() );
#endif // _DEBUG

  std::shared_ptr<JavaClass> pTargetClass = pVirtualMachineState->InitialiseClass( *pClassName );
  const size_t c_NumberOfInterfacesFound = pTargetClass->GetInterfacesCount();

  boost::intrusive_ptr<ObjectReference> pResults = pVirtualMachineState->CreateArray( e_JavaArrayTypes::Reference, c_NumberOfInterfacesFound );

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  pLogger->LogDebug( "\tPopulating interface array.\n" );
#endif // _DEBUG

  size_t interfaceCounter = 0;
  for ( size_t i = 0; i < pTargetClass->GetInterfacesCount(); ++i )
  {
    // Make sure the class is loaded.
    pVirtualMachineState->InitialiseClass( pTargetClass->GetInterfaceName( i ) );

    boost::intrusive_ptr<ObjectReference> pJavaLangClass = pVirtualMachineState->CreateJavaLangClassFromClassName( pTargetClass->GetInterfaceName( i ) );
    JVMX_ASSERT( !pJavaLangClass->IsNull() );

    pResults->GetContainedArray()->SetAt( interfaceCounter, pJavaLangClass.get() );
    ++interfaceCounter;

#ifdef _DEBUG
    JVMX_ASSERT( interfaceCounter <= c_NumberOfInterfacesFound );
#endif // _DEBUG
  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  pVirtualMachineState->LogOperandStack();
  pVirtualMachineState->LogCallStack();

  pLogger->LogDebug( "\tinterface array construction completed.\n" );
#endif // _DEBUG

  return JNIEnvInternal::ConvertArrayPointerToJArray( pVirtualMachineState, pResults );
}

jobject JNICALL HelperVMClass::java_lang_VMClass_getSuperclass( JNIEnv *pEnv, jobject obj, jobject classObject )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMClass_getSuperclass()\n" );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  boost::intrusive_ptr<ObjectReference> pObject = JNIEnvInternal::ConvertJObjectToObjectPointer( classObject );
  if ( e_JavaVariableTypes::Array == pObject->GetVariableType() )
  {
    return JNIEnvInternal::ConvertObjectPointerToJObject( pVirtualMachineState, pVirtualMachineState->CreateJavaLangClassFromClassName( JavaString::FromCString( u"java/lang/Object" ) ) );
  }

  boost::intrusive_ptr<JavaString> pClassName = GetJavaLangClassName( pObject );
  std::shared_ptr<JavaClass> pClass = pVirtualMachineState->InitialiseClass( *pClassName );

  if ( pClass->GetSuperClassName()->IsEmpty() )
  {
    return JNIEnvInternal::ConvertNullPointerToJObject();
  }

  std::shared_ptr<JavaClass> pSuperClass = pVirtualMachineState->InitialiseClass( *pClass->GetSuperClassName() );
  //if ( !pSuperClass->IsInitialsed() )
  //{
  //  pVirtualMachineState->InitialiseClass( *pSuperClass->GetName() );
  //}

  return JNIEnvInternal::ConvertObjectPointerToJObject( pVirtualMachineState, pVirtualMachineState->CreateJavaLangClassFromClassName( *pSuperClass->GetName() ) );
}

jboolean JNICALL HelperVMClass::java_lang_VMClass_isAssignableFrom(JNIEnv* pEnv, jobject obj, jclass callingClass, jclass classToCheck)
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get("Logger");
  pLogger->LogDebug("*** Inside native Method: java_lang_VMClass_isAssignableFrom(%p, %p)\n", callingClass, classToCheck);
#endif // _DEBUG

  JNIEnvExported* pInternal = reinterpret_cast<JNIEnvExported*>(pEnv);
  IVirtualMachineState* pVirtualMachineState = reinterpret_cast<IVirtualMachineState*>(pInternal->m_pInternal);

  if (nullptr == classToCheck)
  {
    pInternal->ThrowNew(pEnv, pInternal->FindClass(pEnv, "java/io/NullPointerException"), "Parameter was null.");
    return JNI_FALSE;
  }

  boost::intrusive_ptr<ObjectReference> pObjectCallingClass = JNIEnvInternal::ConvertJObjectToObjectPointer(callingClass);
  boost::intrusive_ptr<JavaString> pCallingClassName = GetJavaLangClassName(pObjectCallingClass);

  boost::intrusive_ptr<ObjectReference> pObjectClassToCheck = JNIEnvInternal::ConvertJObjectToObjectPointer(classToCheck);
  if (pObjectClassToCheck->IsNull())
  {
    pInternal->ThrowNew(pEnv, pInternal->FindClass(pEnv, "java/io/NullPointerException"), "Parameter was null.");
    return JNI_FALSE;
  }
  boost::intrusive_ptr<JavaString> pClassToCheckName = GetJavaLangClassName(pObjectClassToCheck);

  std::shared_ptr<JavaClass> pCallingClassValue = pVirtualMachineState->FindClass(*pCallingClassName);
  std::shared_ptr<JavaClass> pClassToCheckValue = pVirtualMachineState->FindClass(*pClassToCheckName);

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  pLogger->LogDebug("*** Classes To check: callingClass: %s classToCheck: %s\n", pCallingClassName->ToUtf8String().c_str(), pClassToCheckName->ToUtf8String().c_str());
#endif // _DEBUG

  if (pCallingClassValue->GetName() == pClassToCheckValue->GetName())
  {
    return JNI_TRUE;
  }

  if (pCallingClassValue->IsInterface())
  {
    if (HelperClasses::DoesClassImplementInterface(pVirtualMachineState, pClassToCheckValue, pCallingClassName.get()))
    {
      return JNI_TRUE;
    }
  }
  else
  {
    if (HelperClasses::IsSuperClassOf(pVirtualMachineState, pCallingClassName.get(), pClassToCheckName.get()))
    {
      return JNI_TRUE;
    }
  }

  return JNI_FALSE;
}

jboolean JNICALL HelperVMClass::java_lang_VMClass_isInstance(JNIEnv* pEnv, jobject obj, jclass callingClass, jobject objectToCheck)
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get("Logger");
  pLogger->LogDebug("*** Inside native Method: java_lang_VMClass_isInstance(%p, %p)\n", callingClass, objectToCheck);
#endif // _DEBUG

  JNIEnvExported* pInternal = reinterpret_cast<JNIEnvExported*>(pEnv);
  IVirtualMachineState* pVirtualMachineState = reinterpret_cast<IVirtualMachineState*>(pInternal->m_pInternal);

  if (nullptr == objectToCheck)
  {
    pInternal->ThrowNew(pEnv, pInternal->FindClass(pEnv, "java/io/NullPointerException"), "Parameter was null.");
    return JNI_FALSE;
  }

  boost::intrusive_ptr<ObjectReference> pObjectCallingClass = JNIEnvInternal::ConvertJObjectToObjectPointer(callingClass);
  boost::intrusive_ptr<JavaString> pCallingClassName = GetJavaLangClassName(pObjectCallingClass);

  boost::intrusive_ptr<ObjectReference> pObjectToCheck = JNIEnvInternal::ConvertJObjectToObjectPointer(objectToCheck);
  if (pObjectToCheck->IsNull())
  {
    pInternal->ThrowNew(pEnv, pInternal->FindClass(pEnv, "java/io/NullPointerException"), "Parameter was null.");
    return JNI_FALSE;
  }

  if (e_JavaVariableTypes::Array == pObjectToCheck->GetVariableType())
  {
    if (e_JavaVariableTypes::Array != pObjectCallingClass->GetVariableType())
    {
      return JNI_FALSE;
    }

    return (pObjectToCheck->GetContainedArray()->GetContainedType()) == (pObjectCallingClass->GetContainedArray()->GetContainedType());
  }

  boost::intrusive_ptr<JavaString> pClassToCheckName = pObjectToCheck->GetContainedObject()->GetClass()->GetName();

  std::shared_ptr<JavaClass> pCallingClassValue = pVirtualMachineState->FindClass(*pCallingClassName);
  std::shared_ptr<JavaClass> pClassToCheckValue = pVirtualMachineState->FindClass(*pClassToCheckName);

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  pLogger->LogDebug("*** Classes To check: callingClass: %s classToCheck: %s\n", pCallingClassName->ToUtf8String().c_str(), pClassToCheckName->ToUtf8String().c_str());
#endif // _DEBUG

  if (*pCallingClassValue->GetName() == *pClassToCheckValue->GetName())
  {
    return JNI_TRUE;
  }

  if (pCallingClassValue->IsInterface())
  {
    return HelperClasses::DoesClassImplementInterface(pVirtualMachineState, pClassToCheckValue, pCallingClassName.get());
  }
  
  return HelperClasses::IsSuperClassOf(pVirtualMachineState, pCallingClassName.get(), pClassToCheckName.get());
}


boost::intrusive_ptr<ObjectReference> HelperVMClass::InitialiseNewConstructorObject( boost::intrusive_ptr<ObjectReference> pVMConstructor, IVirtualMachineState *pVirtualMachineState, std::shared_ptr<JavaClass> pConstructorClass, std::shared_ptr<MethodInfo> pConstructorToExecute )
{
  boost::intrusive_ptr<ObjectReference> pNewConstructor = pVirtualMachineState->CreateObject( pConstructorClass );

  pVirtualMachineState->PushOperand( pNewConstructor );
  pVirtualMachineState->PushOperand( pVMConstructor );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  pVirtualMachineState->LogOperandStack();
#endif // _DEBUG && defined(JVMX_LOG_VERBOSE)

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  pVirtualMachineState->LogOperandStack();
  pVirtualMachineState->LogLocalVariables();

  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "\tExecuting %s\n", pConstructorToExecute->GetFullName().ToUtf8String().c_str() );
#endif // _DEBUG && defined(JVMX_LOG_VERBOSE)

  pVirtualMachineState->ExecuteMethod( *pConstructorToExecute->GetClass()->GetName(), *pConstructorToExecute->GetName(), *pConstructorToExecute->GetType(), pConstructorToExecute );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  pVirtualMachineState->LogOperandStack();
#endif

  return pNewConstructor;
}

boost::intrusive_ptr<ObjectReference> HelperVMClass::InitialiseNewVMConstructor( size_t i, boost::intrusive_ptr<ObjectReference> pDeclaringClass, IVirtualMachineState *pVirtualMachineState, std::shared_ptr<JavaClass> pVMConstructor, std::shared_ptr<MethodInfo> pConstructorToExecute )
{
  boost::intrusive_ptr< JavaInteger > pSlot = new JavaInteger( JavaInteger::FromHostInt32( i ) );
  boost::intrusive_ptr<ObjectReference> pNewConstructor = pVirtualMachineState->CreateObject( pVMConstructor );

  pVirtualMachineState->PushOperand( pNewConstructor );
  pVirtualMachineState->PushOperand( pDeclaringClass );
  pVirtualMachineState->PushOperand( pSlot );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  pVirtualMachineState->LogOperandStack();
#endif

  //  std::vector<boost::intrusive_ptr<IJavaVariableType> > paramArray = pVirtualMachineState->PopulateParameterArrayFromOperandStack( pConstructorToExecute );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  pVirtualMachineState->LogOperandStack();
  pVirtualMachineState->LogLocalVariables();

  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "\tExecuting %s\n", pConstructorToExecute->GetFullName().ToUtf8String().c_str() );
#endif // _DEBUG

  pVirtualMachineState->ExecuteMethod( *pConstructorToExecute->GetClass()->GetName(), *pConstructorToExecute->GetName(), *pConstructorToExecute->GetType(), pConstructorToExecute );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  pVirtualMachineState->LogOperandStack();
#endif

  return pNewConstructor;
}

boost::intrusive_ptr<JavaString> HelperVMClass::GetJavaLangClassName( boost::intrusive_ptr<ObjectReference> pObject )
{
  // This method is is used to get the name of the class which the java/Lang/Class represents.
#if defined (_DEBUG)
  auto name = pObject->GetContainedObject()->GetClass()->GetName();
  JVMX_ASSERT( *name == JavaString::FromCString(u"java/lang/Class"));
#endif 

  boost::intrusive_ptr<JavaString> pClassName = boost::dynamic_pointer_cast<JavaString>( pObject->GetContainedObject()->GetJVMXFieldByName( c_SyntheticField_ClassName ) );
  if ( nullptr == pClassName )
  {
    throw InvalidStateException( "Expected Name to be valid object." );
  }
  return pClassName;
}

size_t HelperVMClass::CountClassConstructors( std::shared_ptr<JavaClass> pClassFile )
{
  size_t numberOfConstructorsFound = 0;
  for ( size_t i = 0; i < pClassFile->GetMethodCount(); ++i )
  {
    if ( c_InstanceInitialisationMethodName == *pClassFile->GetMethodByIndex( i )->GetName() )
    {
      ++ numberOfConstructorsFound;
    }
  }

  return numberOfConstructorsFound;
}

boost::intrusive_ptr<ObjectReference> HelperVMClass::InitialiseNewVMField( size_t i, boost::intrusive_ptr<ObjectReference> pDeclaringClass, const JavaString &fieldName, IVirtualMachineState *pVirtualMachineState, std::shared_ptr<JavaClass> pVMField, std::shared_ptr<MethodInfo> pConstructorToExecute )
{
  boost::intrusive_ptr< JavaInteger > pSlot = new JavaInteger( JavaInteger::FromHostInt32( i ) );
  boost::intrusive_ptr<ObjectReference> pNewVMField = pVirtualMachineState->CreateObject( pVMField );

  pVirtualMachineState->PushOperand( pNewVMField );
  pVirtualMachineState->PushOperand( pDeclaringClass );
  pVirtualMachineState->PushOperand( pVirtualMachineState->CreateStringObject( fieldName ) );
  pVirtualMachineState->PushOperand( pSlot );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  pVirtualMachineState->LogOperandStack();
  pVirtualMachineState->LogLocalVariables();

#endif // _DEBUG && defined(JVMX_LOG_VERBOSE)

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "\tExecuting %s\n", pConstructorToExecute->GetFullName().ToUtf8String().c_str() );
#endif // _DEBUG

  pVirtualMachineState->ExecuteMethod( *pConstructorToExecute->GetClass()->GetName(), *pConstructorToExecute->GetName(), *pConstructorToExecute->GetType(), pConstructorToExecute );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  pVirtualMachineState->LogOperandStack();
#endif

  return pNewVMField;
}

boost::intrusive_ptr<ObjectReference> HelperVMClass::InitialiseNewFieldObject( boost::intrusive_ptr<ObjectReference> pVMField, IVirtualMachineState *pVirtualMachineState, std::shared_ptr<JavaClass> pFieldClass, std::shared_ptr<MethodInfo> pConstructorToExecute )
{
  boost::intrusive_ptr<ObjectReference> pNewField = pVirtualMachineState->CreateObject( pFieldClass );

  pVirtualMachineState->PushOperand( pNewField );
  pVirtualMachineState->PushOperand( pVMField );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  pVirtualMachineState->LogOperandStack();
  pVirtualMachineState->LogLocalVariables();
#endif // _DEBUG && defined(JVMX_LOG_VERBOSE)

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "\tExecuting %s\n", pConstructorToExecute->GetFullName().ToUtf8String().c_str() );
#endif // _DEBUG && defined(JVMX_LOG_VERBOSE)

  pVirtualMachineState->ExecuteMethod( *pConstructorToExecute->GetClass()->GetName(), *pConstructorToExecute->GetName(), *pConstructorToExecute->GetType(), pConstructorToExecute );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  pVirtualMachineState->LogOperandStack();
#endif

  return pNewField;
}

