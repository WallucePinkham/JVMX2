#include <thread>
#include <cmath>
#include <memory>

#include <direct.h>

#include <wallaroo/registered.h>
#include <wallaroo/catalog.h>

#include "GlobalConstants.h"

#include "ILogger.h"
#include "IVirtualMachineState.h"
#include "IClassLibrary.h"

#include "InvalidStateException.h"
#include "InvalidArgumentException.h"
#include "AssertionFailedException.h"

#include "OsFunctions.h"
#include "ObjectReference.h"

#include "JavaClass.h"
#include "DefaultClassLoader.h"

#include "JavaNativeInterface.h"
//#include "jni_internal.h"

#include "HelperVMRuntime.h"
#include "HelperVMSystem.h"
#include "HelperVMClass.h"
#include "HelperVMThread.h"
#include "HelperVMDouble.h"
#include "HelperVMChannel.h"
#include "HelperTypes.h"
#include "HelperConversion.h"

#include "VirtualMachine.h"

#include "FileLogger.h"
#include "ConsoleLogger.h"
#include "AgregateLogger.h"

#include "CheneyGarbageCollector.h"

#include "BasicClassLibrary.h"
#include "BasicExecutionEngine.h"
#include "BasicVirtualMachineState.h"
#include "DefaultJavaLangClassList.h"
#include "NativeLibraryContainer.h"
#include "ObjectRegistryLocalMachine.h"
#include "FileSearchPathCollection.h"
#ifdef REDIS_SUPPORT
#include "ObjectRegistryRedis.h"
#include "RedisGarbageCollector.h"
#endif // REDIS_SUPPORT
#include "GlobalCatalog.h"

WALLAROO_REGISTER( FileLogger, const JVMX_ANSI_CHAR_TYPE * );
WALLAROO_REGISTER( ConsoleLogger );
WALLAROO_REGISTER( AgregateLogger );

//WALLAROO_REGISTER( MallocFreeMemoryManager );
//WALLAROO_REGISTER( SimpleGreedyMemoryManager );
WALLAROO_REGISTER( ObjectRegistryLocalMachine );

WALLAROO_REGISTER( CheneyGarbageCollector, std::shared_ptr<ThreadManager>, size_t );
WALLAROO_REGISTER( BasicExecutionEngine );
WALLAROO_REGISTER( JavaNativeInterface );
WALLAROO_REGISTER( DefaultJavaLangClassList );
WALLAROO_REGISTER( ThreadManager );
WALLAROO_REGISTER( NativeLibraryContainer );
WALLAROO_REGISTER(FileSearchPathCollection);
#ifdef REDIS_SUPPORT
WALLAROO_REGISTER( RedisGarbageCollector, const std::string &, size_t );
WALLAROO_REGISTER( ObjectRegistryRedis );
#endif

//WALLAROO_REGISTER( VirtualMachine );

static const JavaString c_ProgramStartMethodName = JavaString::FromCString( JVMX_T( "main" ) );
static const JavaString c_ProgramStartMethodType = JavaString::FromCString( JVMX_T( "([Ljava/lang/String;)V" ) );
static const JavaString c_SystemClassInitialisationMethodName = JavaString::FromCString( JVMX_T( "initializeSystemClass" ) );

#ifdef _WIN32
const JVMX_CHAR_TYPE c_PathSeparator = JVMX_T( '\\' );
#else
const JVMX_CHAR_TYPE c_PathSeparator = JVMX_T('/');
#endif

static const size_t c_DefaultGarbageCollectionPoolSize = ( 1024 * 1024 ) * 100;

extern const JavaString c_ClassInitialisationMethodType;
extern const JavaString c_ClassInitialisationMethodName;
extern const JavaString c_InstanceInitialisationMethodName;
extern const JavaString c_InstanceInitialisationMethodType;
extern const JavaString c_SyntheticField_ClassName;
extern const JavaString c_JavaLangClassName;

VirtualMachine::VirtualMachine()
{}

void VirtualMachine::Run( const JVMX_CHAR_TYPE *pFileName, const std::shared_ptr<IVirtualMachineState> &pInitialState, bool userCode )
{
  try
  {
    if (userCode)
    {
        pInitialState->SetUserCodeStarted();
    }
    auto className = GetClassNameFromFileName( pFileName );

    JavaString fileNameString = JavaString::FromCString(pFileName);
    JavaString path = JavaString::EmptyString();
    
    size_t slashPos = fileNameString.FindLast(c_PathSeparator);
    if (JavaString::NotFound() != slashPos)
    {
        path = fileNameString.SubString(0, slashPos + 1);
    }

    auto clazz = pInitialState->LoadClass( className, path );
    pInitialState->InitialiseClass( className );

    //Because this is main(), we need to push an array of strings.
    pInitialState->PushOperand( pInitialState->CreateArray( e_JavaArrayTypes::Reference, 0 ) );

    pInitialState->Execute( className, c_ProgramStartMethodName, c_ProgramStartMethodType );
  }
  catch ( JVMXException &ex )
  {
    m_pLogger->LogError( __FUNCTION__ " - Exception thrown from running virtual machine." );
    m_pLogger->LogError( "\tException Detail:" );
    m_pLogger->LogError( ex.what() );
  }
}

void VirtualMachine::LoadFile( const JVMX_CHAR_TYPE *pFileName )
{
  if ( nullptr == pFileName )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Filename is empty." );
  }

  DefaultClassLoader loader;
  std::shared_ptr<JavaClass> classFile = loader.LoadClass( pFileName );
  m_pRuntimeConstantPool->AddClass( classFile );
}

JavaString VirtualMachine::GetClassNameFromFileName( const JVMX_CHAR_TYPE *pFileName )
{
  JavaString fileName = JavaString::FromCString( pFileName );
  size_t dotPos = fileName.FindLast( JVMX_T( '.' ) );
  if ( JavaString::NotFound() == dotPos )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Could not determine class name from filename." );
  }

  JavaString className = fileName.SubString( 0, dotPos );

  size_t slashPos = className.FindLast(c_PathSeparator);
  if (JavaString::NotFound() != slashPos)
  {
      className = className.SubString(slashPos + 1);
  }

  return className;
}

void VirtualMachine::InitialiseClass( const JVMX_CHAR_TYPE *pClassName, const std::shared_ptr<IVirtualMachineState> &pInitialState )
{
  try
  {
    pInitialState->Execute( JavaString::FromCString( pClassName ), c_ClassInitialisationMethodName, c_ClassInitialisationMethodType );
  }
  catch ( JVMXException &ex )
  {
      std::string msg(__FUNCTION__ " - Exception thrown from while initializing class: ");
      std::string classNameUtf8 = HelperConversion::ConvertJVMXCharToUtf8(pClassName);
      msg.append(classNameUtf8) ;

    m_pLogger->LogError( msg.c_str() );
    m_pLogger->LogError( "\tException Detail: %s", ex.what() );
    throw;
  }
}

void VirtualMachine::RegisterNativeMethods( std::shared_ptr<JavaNativeInterface> pJNI )
{
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMRuntime_insertSystemProperties" ), HelperVMRuntime::java_lang_VMRuntime_insertSystemProperties );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMRuntime_mapLibraryName" ), HelperVMRuntime::Java_java_lang_VMRuntime_mapLibraryName );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMRuntime_nativeLoad" ), HelperVMRuntime::java_lang_VMRuntime_nativeLoad );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMRuntime_runFinalizationForExit" ), HelperVMRuntime::java_lang_VMRuntime_runFinalizationForExit );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMRuntime_exit" ), HelperVMRuntime::java_lang_VMRuntime_exit );

  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMSystem_identityHashCode" ), HelperVMSystem::java_lang_VMSystem_identityHashCode );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMSystem_arraycopy" ), HelperVMSystem::java_lang_VMSystem_arraycopy );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMSystem_isWordsBigEndian" ), HelperVMSystem::java_lang_VMSystem_isWordsBigEndian );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMSystem_currentTimeMillis" ), HelperVMSystem::java_lang_VMSystem_currentTimeMillis );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_gnu_classpath_VMSystemProperties_preInit" ), HelperVMSystem::gnu_classpath_VMSystemProperties_preInit );

  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMClass_forName" ), HelperVMClass::java_lang_VMClass_forName );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMClass_getName" ), HelperVMClass::java_lang_VMClass_getName );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMClass_getDeclaredConstructors" ), HelperVMClass::java_lang_VMClass_getDeclaredConstructors );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMClass_isArray" ), HelperVMClass::java_lang_VMClass_isArray );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMClass_isPrimitive" ), HelperVMClass::java_lang_VMClass_isPrimitive );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMClass_getClassLoader" ), HelperVMClass::java_lang_VMClass_getClassLoader );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMClass_getModifiers" ), HelperVMClass::java_lang_VMClass_getModifiers );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMClass_getComponentType" ), HelperVMClass::java_lang_VMClass_getComponentType );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMClass_getDeclaredFields" ), HelperVMClass::java_lang_VMClass_getDeclaredFields );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMClass_getInterfaces" ), HelperVMClass::java_lang_VMClass_getInterfaces );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMClass_getSuperclass" ), HelperVMClass::java_lang_VMClass_getSuperclass );

  //(Java_java_lang_VMClass_getModifiers)

  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMClassLoader_getPrimitiveClass" ), VirtualMachine::java_lang_VMClassLoader_getPrimitiveClass );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMSecurityManager_currentClassLoader" ), VirtualMachine::java_lang_VMSecurityManager_currentClassLoader );

  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMObject_clone" ), VirtualMachine::java_lang_VMObject_clone );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMObject_wait" ), VirtualMachine::java_lang_VMObject_wait );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMObject_getClass" ), VirtualMachine::java_lang_VMObject_getClass );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMObject_notifyAll" ), VirtualMachine::java_lang_VMObject_notifyAll );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMObject_notify" ), VirtualMachine::java_lang_VMObject_notify );

  pJNI->RegisterFunction( JavaString::FromCString( u"Java_gnu_java_nio_VMChannel_initIDs" ), HelperVMChannel::gnu_java_nio_VMChannel_initIDs );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_gnu_java_nio_VMChannel_stdin_fd" ), HelperVMChannel::gnu_java_nio_VMChannel_stdin_fd );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_gnu_java_nio_VMChannel_stdout_fd" ), HelperVMChannel::gnu_java_nio_VMChannel_stdout_fd );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_gnu_java_nio_VMChannel_stderr_fd" ), HelperVMChannel::gnu_java_nio_VMChannel_stderr_fd );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_gnu_java_nio_VMChannel_write" ), HelperVMChannel::gnu_java_nio_VMChannel_write );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_gnu_java_nio_VMChannel_close" ), HelperVMChannel::gnu_java_nio_VMChannel_close );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_gnu_java_nio_VMChannel_open" ), HelperVMChannel::gnu_java_nio_VMChannel_open );

  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMThread_currentThread" ), HelperVMThread::java_lang_VMThread_currentThread );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMThread_start" ), HelperVMThread::java_lang_VMThread_start );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMThread_sleep" ), HelperVMThread::java_lang_VMThread_sleep );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMThread_suspend" ), HelperVMThread::java_lang_VMThread_suspend );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMThread_suspend" ), HelperVMThread::java_lang_VMThread_resume );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMThread_interrupted" ), HelperVMThread::java_lang_VMThread_interrupted );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMThread_interrupt" ), HelperVMThread::java_lang_VMThread_interrupt );

  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMDouble_initIDs" ), HelperVMDouble::java_lang_Double_initIDs );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMDouble_toString" ), HelperVMDouble::java_lang_Double_toString );

  //pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_reflect_Constructor_constructNative" ), VirtualMachine::java_lang_reflect_Constructor_constructNative );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_reflect_Constructor_getModifiers" ), VirtualMachine::java_lang_reflect_Constructor_getModifiers );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_reflect_VMConstructor_construct" ), VirtualMachine::java_lang_reflect_VMConstructor_construct );

  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_reflect_VMConstructor_getParameterTypes" ), VirtualMachine::java_lang_reflect_VMConstructor_getParameterTypes );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_reflect_VMConstructor_getModifiersInternal" ), VirtualMachine::java_lang_reflect_VMConstructor_getModifiersInternal );

  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_io_VMFile_isDirectory" ), VirtualMachine::java_io_VMFile_isDirectory );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_io_VMFile_exists" ), VirtualMachine::java_io_VMFile_exists );

  pJNI->RegisterFunction( JavaString::FromCString( u"Java_gnu_classpath_VMStackWalker_getClassContext" ), VirtualMachine::gnu_classpath_VMStackWalker_getClassContext );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_gnu_classpath_VMStackWalker_getClassLoader" ), VirtualMachine::gnu_classpath_VMStackWalker_getClassLoader );

  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMThrowable_fillInStackTrace" ), VirtualMachine::java_lang_VMThrowable_fillInStackTrace );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMThrowable_getStackTrace" ), VirtualMachine::Java_java_lang_VMThrowable_getStackTrace );

  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMMath_log" ), VirtualMachine::java_lang_VMMath_log );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_VMMath_exp" ), VirtualMachine::java_lang_VMMath_exp );

  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_reflect_VMField_get" ), VirtualMachine::java_lang_reflect_VMField_get );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_reflect_VMField_getModifiersInternal" ), VirtualMachine::java_lang_reflect_VMField_getModifiersInternal );
  pJNI->RegisterFunction( JavaString::FromCString( u"Java_java_lang_reflect_VMField_getType" ), VirtualMachine::java_lang_reflect_VMField_getType );
}

jobject JNICALL VirtualMachine::java_lang_VMClassLoader_getPrimitiveClass( JNIEnv *pEnv, jobject obj, jchar typeAsChar )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMClassLoader_getPrimitiveClass(%u)", typeAsChar );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  boost::intrusive_ptr<ObjectReference> pResult = GetPrimitiveClass( typeAsChar, pVirtualMachineState );

  //JavaClass *pClass = reinterpret_cast<JavaClass *>(clazz);
  return JNIEnvInternal::ConvertObjectPointerToJObject( pVirtualMachineState, pResult );
}

boost::intrusive_ptr<ObjectReference> VirtualMachine::GetPrimitiveClass( jstring typeAsString, IVirtualMachineState *pVirtualMachineState )
{
  boost::intrusive_ptr<ObjectReference> pTypeNameStringObject = JNIEnvInternal::ConvertJObjectToObjectPointer( typeAsString );
  JavaString finalStringValue = HelperTypes::ExtractValueFromStringObject( pTypeNameStringObject );

  const char *pFinalClassName = GetClassNameFromType( finalStringValue );

  std::shared_ptr<JavaClass> pClass = pVirtualMachineState->LoadClass( JavaString::FromCString( pFinalClassName ) );
  return pVirtualMachineState->CreateJavaLangClassFromClassName( pClass->GetName() );
}

boost::intrusive_ptr<ObjectReference> VirtualMachine::GetPrimitiveClass( jchar chr, IVirtualMachineState *pVirtualMachineState )
{
  JavaString longTypeName = JavaString::EmptyString();
  switch ( chr )
  {
    case 'Z':
      longTypeName = JavaString::FromCString( "boolean" );
      break;
    case 'B':
      longTypeName = JavaString::FromCString( "byte" );
      break;
    case 'C':
      longTypeName = JavaString::FromCString( "char" );
      break;
    case 'D':
      longTypeName = JavaString::FromCString( "double" );
      break;
    case 'F':
      longTypeName = JavaString::FromCString( "float" );
      break;
    case 'I':
      longTypeName = JavaString::FromCString( "int" );
      break;
    case 'J':
      longTypeName = JavaString::FromCString( "long" );
      break;
    case 'S':
      longTypeName = JavaString::FromCString( "short" );
      break;
    case 'V':
      longTypeName = JavaString::FromCString( "void" );
      break;
    default:
      throw InvalidArgumentException( __FUNCTION__ " - Type Not found." );
  }

  const char *pFinalClassName = GetClassNameFromType( longTypeName );

  std::shared_ptr<JavaClass> pClass = pVirtualMachineState->LoadClass( JavaString::FromCString( pFinalClassName ) );
  return pVirtualMachineState->CreateJavaLangClassFromClassName( pClass->GetName() );
}

const char *VirtualMachine::GetClassNameFromType( JavaString finalStringValue )
{
  const char *pFinalClassName = nullptr;
  if ( JavaString::FromCString( "boolean" ) == finalStringValue )
  {
    pFinalClassName = "java/lang/Boolean";
  }
  else if ( JavaString::FromCString( "byte" ) == finalStringValue )
  {
    pFinalClassName = "java/lang/Byte";
  }
  else if ( JavaString::FromCString( "char" ) == finalStringValue )
  {
    pFinalClassName = "java/lang/Character";
  }
  else if ( JavaString::FromCString( "double" ) == finalStringValue )
  {
    pFinalClassName = "java/lang/Double";
  }
  else if ( JavaString::FromCString( "float" ) == finalStringValue )
  {
    pFinalClassName = "java/lang/Float";
  }
  else if ( JavaString::FromCString( "int" ) == finalStringValue )
  {
    pFinalClassName = "java/lang/Integer";
  }
  else if ( JavaString::FromCString( "long" ) == finalStringValue )
  {
    pFinalClassName = "java/lang/Long";
  }
  else if ( JavaString::FromCString( "short" ) == finalStringValue )
  {
    pFinalClassName = "java/lang/Short";
  }
  else if ( JavaString::FromCString( "void" ) == finalStringValue )
  {
    pFinalClassName = "java/lang/Void";
  }
  else
  {
    // TODO: Throw Java Exception Here
    throw InvalidArgumentException( __FUNCTION__ " - Type Not found." );
  }

  return pFinalClassName;
}

jobject JNICALL VirtualMachine::java_lang_VMSecurityManager_currentClassLoader( JNIEnv *pEnv, jobject obj )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMSecurityManager_currentClassLoader" );
#endif // _DEBUG

  return JNIEnvInternal::ConvertNullPointerToJObject();
}

jobject JNICALL VirtualMachine::java_lang_VMObject_clone( JNIEnv *pEnv, jobject obj, jobject objectToClone )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMObject_clone(objectToClone=0x%p)", objectToClone );
#endif // _DEBUG

  boost::intrusive_ptr<ObjectReference> pObjectToClone = JNIEnvInternal::ConvertJObjectToObjectPointer( objectToClone );
  jmethodID emptyMethodId = nullptr;

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  std::shared_ptr<JavaClass> pClass = pVirtualMachineState->LoadClass( *pObjectToClone->GetContainedObject()->GetClass()->GetName() );
  if ( !pClass->IsInitialsed() )
  {
    pVirtualMachineState->InitialiseClass( *pClass->GetName() );
  }

  //jobject newObject = pEnv->NewObject( pEnv, static_cast<jclass>(JNIEnvInternal::ConvertObjectPointerToJObject( pJavaLangClass )), emptyMethodId );
  boost::intrusive_ptr<ObjectReference> pNewOject = pVirtualMachineState->CreateObject( pClass );

  pNewOject->CloneOther( *pObjectToClone );

  return JNIEnvInternal::ConvertObjectPointerToJObject( pVirtualMachineState, pNewOject );
}

void JNICALL VirtualMachine::java_lang_VMObject_wait( JNIEnv *pEnv, jobject obj, jobject objectToWaitFor, jlong ms, jint ns )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMObject_wait(objectToWaitFor=0x%p, ms=%lld, ns=%ld)", objectToWaitFor, ms, ns );
#endif // _DEBUG

  boost::intrusive_ptr<ObjectReference> pObjectToWaitFor = JNIEnvInternal::ConvertJObjectToObjectPointer( objectToWaitFor );

#if defined (_DEBUG) && defined (JVMX_LOG_VERBOSE)
  pLogger->LogDebug( "Waiting for %s", pObjectToWaitFor->GetContainedObject()->GetClass()->GetName()->ToUtf8String().c_str() );
#endif

  pObjectToWaitFor->GetContainedObject()->Wait( JavaLong::FromHostInt64( ms ), JavaInteger::FromHostInt32( ns ) );
}

jobject JNICALL VirtualMachine::java_lang_VMObject_getClass( JNIEnv *pEnv, jobject obj, jobject objectToGetClassFor )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMObject_getClass(%p)", objectToGetClassFor );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  boost::intrusive_ptr<ObjectReference> pObject = JNIEnvInternal::ConvertJObjectToObjectPointer( objectToGetClassFor );
  boost::intrusive_ptr<JavaString> pClassName = pObject->GetContainedObject()->GetClass()->GetName();
  boost::intrusive_ptr<ObjectReference> pJavaLangClass = pVirtualMachineState->CreateJavaLangClassFromClassName( pClassName );

  return JNIEnvInternal::ConvertObjectPointerToJObject( pVirtualMachineState, pJavaLangClass );
}

void JNICALL VirtualMachine::java_lang_VMObject_notifyAll( JNIEnv *pEnv, jobject obj, jobject objectToNotifyOn )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMObject_notifyAll(%p)", objectToNotifyOn );
#endif // _DEBUG

  boost::intrusive_ptr<ObjectReference> pObject = JNIEnvInternal::ConvertJObjectToObjectPointer( objectToNotifyOn );
  pObject->GetContainedObject()->NotifyAll();
}

void JNICALL VirtualMachine::java_lang_VMObject_notify( JNIEnv *pEnv, jobject obj, jobject objectToNotifyOn )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMObject_notifyAll(%p)", objectToNotifyOn );
#endif // _DEBUG

  boost::intrusive_ptr<ObjectReference> pObject = JNIEnvInternal::ConvertJObjectToObjectPointer( objectToNotifyOn );
  pObject->GetContainedObject()->NotifyOne();
}

// jobject JNICALL VirtualMachine::java_lang_reflect_Constructor_constructNative( JNIEnv *pEnv, jobject obj, jarray args, jclass declaringClass, jint slot )
// {
//
// }

jboolean JNICALL VirtualMachine::java_io_VMFile_isDirectory( JNIEnv *pEnv, jobject obj, jstring path )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_java_io_VMFile_isDirectory( 0x%p )", path );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  boost::intrusive_ptr<ObjectReference> pStringObject = JNIEnvInternal::ConvertJObjectToObjectPointer( path );

  JavaString stringValue = HelperTypes::ExtractValueFromStringObject( pStringObject );
  if ( OsFunctions::GetInstance().IsDirectory( stringValue.ToUtf8String().c_str() ) )
  {
    return JNI_TRUE;
  }

  return JNI_FALSE;
}

std::unique_ptr<jvalue[]> VirtualMachine::ConvertJArrayToArrayOfObjects( jarray args, TypeParser::ParsedMethodType methodType )
{
  JavaArray *pArray = JNIEnvInternal::ConvertJArrayToArrayPointer( args )->GetContainedArray();
  if ( pArray->IsNull() )
  {
    return nullptr;
  }

  std::unique_ptr<jvalue[]> pResult = std::make_unique< jvalue[] >( pArray->GetNumberOfElements() );

  if ( 0 == pArray->GetNumberOfElements() )
  {
    pResult = nullptr;
    return pResult;
  }

  JVMX_ASSERT( methodType.parameters.size() == pArray->GetNumberOfElements() );
  if ( methodType.parameters.size() != pArray->GetNumberOfElements() )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Number of arguments passed does not match number of arguments for the requested method." );
  }

  for ( size_t index = 0; index < pArray->GetNumberOfElements(); ++index )
  {
    IJavaVariableType *pElementValue = pArray->At( index );
    boost::intrusive_ptr<ObjectReference> ref = new ObjectReference( *dynamic_cast<const ObjectReference *>( pElementValue ) );

    pResult[ index ] = ConvertJavaVariableToJValue( ref, methodType.parameters[index] );
  }

  return pResult;
}

jvalue VirtualMachine::ConvertJavaVariableToJValue( const boost::intrusive_ptr<IJavaVariableType> pElementValue, boost::intrusive_ptr<JavaString> parameterType )
{
  jvalue newValue = { 0 };
  switch ( parameterType->At( 0 ) )
  {
    case c_JavaTypeSpecifierBool:
      {
        if ( pElementValue->GetVariableType() == e_JavaVariableTypes::Bool )
        {
          boost::intrusive_ptr<IJavaVariableType> pBool = boost::dynamic_pointer_cast<JavaBool>( pElementValue );
          if ( pBool->IsNull() )
          {
            throw InvalidArgumentException( __FUNCTION__ " - Exepected bool but received null." );
          }

          newValue.z = boost::dynamic_pointer_cast<JavaBool>( pElementValue )->ToBool();
        }
        else if ( pElementValue->GetVariableType() == e_JavaVariableTypes::Object )
        {
          newValue = HelperTypes::Unbox( pElementValue );
        }
        else
        {
          throw InvalidArgumentException( __FUNCTION__ " - Exepected Bool but received another variable type." );
        }
        break;
      }

    case c_JavaTypeSpecifierChar:
      {
        if ( pElementValue->GetVariableType() == e_JavaVariableTypes::Char )
        {
          boost::intrusive_ptr<IJavaVariableType> pChar = boost::dynamic_pointer_cast<JavaChar>( pElementValue );
          if ( pChar->IsNull() )
          {
            throw InvalidArgumentException( __FUNCTION__ " - Exepected char but received null." );
          }

          newValue.c = boost::dynamic_pointer_cast<JavaChar>( pElementValue )->ToChar16();
        }
        else if ( pElementValue->GetVariableType() == e_JavaVariableTypes::Object )
        {
          newValue = HelperTypes::Unbox( pElementValue );
        }
        else
        {
          throw InvalidArgumentException( __FUNCTION__ " - Exepected char but received another variable type." );
        }
        break;
      }

    case c_JavaTypeSpecifierFloat:
      {
        if ( pElementValue->GetVariableType() == e_JavaVariableTypes::Float )
        {

          boost::intrusive_ptr<IJavaVariableType> pFloat = boost::dynamic_pointer_cast<JavaFloat>( pElementValue );
          if ( pFloat->IsNull() )
          {
            throw InvalidArgumentException( __FUNCTION__ " - Exepected float but received null." );
          }

          newValue.f = boost::dynamic_pointer_cast<JavaFloat>( pElementValue )->ToHostFloat();
        }
        else if ( pElementValue->GetVariableType() == e_JavaVariableTypes::Object )
        {
          newValue = HelperTypes::Unbox( pElementValue );
        }
        else
        {
          throw InvalidArgumentException( __FUNCTION__ " - Exepected float but received another variable type." );
        }
        break;
      }

    case c_JavaTypeSpecifierDouble:
      {
        if ( pElementValue->GetVariableType() == e_JavaVariableTypes::Double )
        {
          boost::intrusive_ptr<IJavaVariableType> pDouble = boost::dynamic_pointer_cast<JavaDouble>( pElementValue );
          if ( pDouble->IsNull() )
          {
            throw InvalidArgumentException( __FUNCTION__ " - Exepected Double but received null." );
          }

          newValue.d = boost::dynamic_pointer_cast<JavaDouble>( pElementValue )->ToHostDouble();
        }
        else if ( pElementValue->GetVariableType() == e_JavaVariableTypes::Object )
        {
          newValue = HelperTypes::Unbox( pElementValue );
        }
        else
        {
          throw InvalidArgumentException( __FUNCTION__ " - Exepected Double but received another variable type." );
        }
        break;
      }

    case c_JavaTypeSpecifierByte:
      {
        if ( pElementValue->GetVariableType() == e_JavaVariableTypes::Byte )
        {
          boost::intrusive_ptr<IJavaVariableType> pByte = boost::dynamic_pointer_cast<JavaByte>( pElementValue );
          if ( pByte->IsNull() )
          {
            throw InvalidArgumentException( __FUNCTION__ " - Exepected Byte but received null." );
          }

          newValue.b = boost::dynamic_pointer_cast<JavaByte>( pElementValue )->ToHostInt8();
        }
        else if ( pElementValue->GetVariableType() == e_JavaVariableTypes::Object )
        {
          newValue = HelperTypes::Unbox( pElementValue );
        }
        else
        {
          throw InvalidArgumentException( __FUNCTION__ " - Exepected Byte but received another variable type." );
        }
        break;
      }

    case c_JavaTypeSpecifierShort:
      {
        if ( pElementValue->GetVariableType() == e_JavaVariableTypes::Short )
        {
          boost::intrusive_ptr<IJavaVariableType> pShort = boost::dynamic_pointer_cast<JavaShort>( pElementValue );
          if ( pShort->IsNull() )
          {
            throw InvalidArgumentException( __FUNCTION__ " - Exepected Short but received null." );
          }

          newValue.s = boost::dynamic_pointer_cast<JavaShort>( pElementValue )->ToHostInt16();
        }
        else if ( pElementValue->GetVariableType() == e_JavaVariableTypes::Object )
        {
          newValue = HelperTypes::Unbox( pElementValue );
        }
        else
        {
          throw InvalidArgumentException( __FUNCTION__ " - Exepected Short but received another variable type." );
        }
        break;
      }

    case c_JavaTypeSpecifierInteger:
      {
        if ( pElementValue->GetVariableType() == e_JavaVariableTypes::Integer )
        {
          boost::intrusive_ptr<IJavaVariableType> pInteger = boost::dynamic_pointer_cast<JavaInteger>( pElementValue );
          if ( pInteger->IsNull() )
          {
            throw InvalidArgumentException( __FUNCTION__ " - Exepected Integer but received null." );
          }

          newValue.i = boost::dynamic_pointer_cast<JavaInteger>( pElementValue )->ToHostInt32();
        }
        else if ( pElementValue->GetVariableType() == e_JavaVariableTypes::Object )
        {
          newValue = HelperTypes::Unbox( pElementValue );
        }
        else
        {
          throw InvalidArgumentException( __FUNCTION__ " - Exepected Integer but received another variable type." );
        }
        break;
      }

    case c_JavaTypeSpecifierLong:
      {
        if ( pElementValue->GetVariableType() == e_JavaVariableTypes::Long )
        {
          boost::intrusive_ptr<IJavaVariableType> pLong = boost::dynamic_pointer_cast<JavaLong>( pElementValue );
          if ( pLong->IsNull() )
          {
            throw InvalidArgumentException( __FUNCTION__ " - Exepected Long but received null." );
          }

          newValue.j = boost::dynamic_pointer_cast<JavaLong>( pElementValue )->ToHostInt64();
        }
        else if ( pElementValue->GetVariableType() == e_JavaVariableTypes::Object )
        {
          newValue = HelperTypes::Unbox( pElementValue );
        }
        else
        {
          throw InvalidArgumentException( __FUNCTION__ " - Exepected Long but received another variable type." );
        }
        break;
      }

    case c_JavaTypeSpecifierArray:
    case c_JavaTypeSpecifierReference:
      newValue.l = JNIEnvInternal::ConvertObjectPointerToJObject( nullptr, boost::dynamic_pointer_cast<ObjectReference>( pElementValue ) );
      break;

    default:
      throw InvalidArgumentException( __FUNCTION__ " - Unknown type. " );
      break;
  }

  return newValue;
}

void VirtualMachine::CreateInitialThreadObject( const std::shared_ptr<IVirtualMachineState> &pInitialState )
{
  static const JavaString c_VMThreadClassName = JavaString::FromCString( JVMX_T( "java/lang/VMThread" ) );

  std::shared_ptr<JavaClass> pVMThreadClass = pInitialState->LoadClass( c_VMThreadClassName );
  if ( !pVMThreadClass->IsInitialsed() )
  {
    pInitialState->InitialiseClass( c_VMThreadClassName );
  }

  boost::intrusive_ptr<ObjectReference> pVMThread = pInitialState->CreateObject( pVMThreadClass );

  std::shared_ptr<JavaClass> pThreadClass = pInitialState->LoadClass( JavaString::FromCString( JVMX_T( "java/lang/Thread" ) ) );
  boost::intrusive_ptr<ObjectReference> pThread = pInitialState->CreateObject( pThreadClass );

  CallThreadConstructor( pInitialState, pThreadClass, pThread, pVMThread );
  CallVMThreadConstructor( pInitialState, pVMThreadClass, pThread, pVMThread );

  //
  //   pThread->SetField( JavaString::FromCString( JVMX_T( "name" ) ), pInitialState->CreateStringObject( "main" ) );
  //   //pThread->SetField( JavaString::FromCString( JVMX_T( "current" ) ), pThread );
  //   pThread->SetField( JavaString::FromCString( JVMX_T( "priority" ) ), new JavaInteger( JavaInteger::FromHostInt32( 10 ) ) );
  //   pThread->SetField( JavaString::FromCString( JVMX_T( "daemon" ) ), new JavaBool( JavaBool::FromBool( false ) ) );

  std::shared_ptr<JavaClass> pThreadGroupClass = pInitialState->LoadClass( JavaString::FromCString( JVMX_T( "java/lang/ThreadGroup" ) ) );
  boost::intrusive_ptr<ObjectReference> pRootGroup = boost::dynamic_pointer_cast<ObjectReference>( pThreadGroupClass->GetFieldByName( JavaString::FromCString( JVMX_T( "root" ) ) )->GetStaticValue() );

  pThread->GetContainedObject()->SetField( JavaString::FromCString( JVMX_T( "group" ) ), pRootGroup );

  m_pThreadManager->AddThread( nullptr, pThread, pInitialState );

  AddThreadToGroup( pInitialState, pThreadGroupClass, pRootGroup, pThread );
  AddToInheritableThreadLocal( pInitialState, pThread );
}

void VirtualMachine::Initialise( const std::string &startingClassfile, const std::shared_ptr<IVirtualMachineState> &pInitialState )
{
  m_pJNI = std::make_shared<JavaNativeInterface>();
  m_pJNI->SetVMState( pInitialState );
  pInitialState->SetJavaNativeInterface( m_pJNI );

  m_pLogger->LogInformation( "JVMX Starting..." );
  m_pLogger->LogInformation( "Current working directory: %s", getcwd( NULL, 0 ) );

  RegisterNativeMethods( m_pJNI );

  m_pLogger->LogInformation( "starting-classfile: %s", startingClassfile.c_str() );

  m_pLogger->LogInformation( "Initializing system classes..." );

  InitialiseClass( JVMX_T( "java/lang/Object" ), pInitialState );
  InitialiseClass( JVMX_T( "java/lang/Class" ), pInitialState );
  InitialiseClass( JVMX_T( "java/lang/String" ), pInitialState );
  InitialiseClass( JVMX_T( "java/lang/Thread" ), pInitialState );
  InitialiseClass( JVMX_T( "java/lang/ThreadGroup" ), pInitialState );

  CreateInitialThreadObject( pInitialState );

  InitialiseClass( JVMX_T( "java/lang/System" ), pInitialState );
  InitialiseClass( JVMX_T( "gnu/java/nio/charset/Provider" ), pInitialState );
  InitialiseClass( JVMX_T( "java/util/Locale" ), pInitialState );
  InitialiseClass( JVMX_T( "java/lang/Character" ), pInitialState );

  // Execute static Provider.provider

  m_pLogger->LogInformation( "System classes initialized." );
  m_pLogger->LogInformation( "Running initial class..." );
}

void VirtualMachine::Stop( const std::shared_ptr<IVirtualMachineState> &pInitialState )
{
  try
  {
#ifdef _DEBUG
    m_pLogger->LogInformation( "JVMX Shutting down..." );
#endif // _DEBUG

    m_pThreadManager->DetachDaemons();

    pInitialState->StartShutdown( 0 );
    m_pThreadManager->JoinAll();

    m_pLogger->LogInformation( "JVMX Shut down." );
  }
  catch ( JVMXException &ex )
  {
    m_pLogger->LogError( __FUNCTION__ " - Exception thrown while shutting down." );
    m_pLogger->LogError( "\tException Detail: %s", ex.what() );
    throw;
  }
}

std::shared_ptr<JavaNativeInterface> VirtualMachine::GetNativeInterface() const
{
  return m_pJNI;
}

ThreadInfo VirtualMachine::ReturnCurrentThreadObject() const
{
  return std::move( m_pThreadManager->GetCurrentThreadInfo() );
}

void VirtualMachine::AddThread( std::shared_ptr<boost::thread> pNewThread, boost::intrusive_ptr<ObjectReference> pObject, const std::shared_ptr<IVirtualMachineState> &pNewState )
{
  m_pThreadManager->AddThread( pNewThread, pObject, pNewState );
}

void VirtualMachine::SetupDependencies( std::shared_ptr<VirtualMachine> pThis )
{
  GlobalCatalog &mainCatalog = GlobalCatalog::GetInstance();

  std::shared_ptr<AgregateLogger> pLogger = std::make_shared<AgregateLogger>();

  std::shared_ptr<ConsoleLogger> pConsoleLogger = std::make_shared<ConsoleLogger>();
  std::shared_ptr<FileLogger> pFileLogger = std::make_shared<FileLogger>( "JVMX_debug.log" );
  pFileLogger->SetLogLevel( 4 );

  pLogger->AddLogger( pConsoleLogger );
  pLogger->AddLogger( pFileLogger );

  m_pLogger = pLogger;
  m_pThreadManager = std::make_shared<ThreadManager>();
  m_pGarbageCollector = std::make_shared<CheneyGarbageCollector>( m_pThreadManager, c_DefaultGarbageCollectionPoolSize );
  //m_pGarbageCollector = std::make_shared<RedisGarbageCollector>( "fpwalpink1" );
  m_pRuntimeConstantPool = std::make_shared<BasicClassLibrary>();
  m_pEngine = std::make_shared<BasicExecutionEngine>();
  m_pJavaLangClassList = std::make_shared<DefaultJavaLangClassList>();
  m_pNativeLibraryContainer = std::make_shared<NativeLibraryContainer>();
  //m_pObjectRegistry = std::make_shared<ObjectRegistryRedis>();
  m_pObjectRegistry = std::make_shared<ObjectRegistryLocalMachine>();
  m_pFileSearchPathCollection = std::make_shared<FileSearchPathCollection>();
  // ************************************************************************************
  // If you want to change a mapping, instantiate the new class above, and change it here
  // that way, the code below can stay the same.
  mainCatalog.Add( "Logger", m_pLogger );
  mainCatalog.Add( "GarbageCollector", m_pGarbageCollector );
  mainCatalog.Add( "ClassLibrary", m_pRuntimeConstantPool );
  mainCatalog.Add( "ExecutionEngine", m_pEngine );
  mainCatalog.Add( "JavaLangClassList", m_pJavaLangClassList );
  mainCatalog.Add( "ThreadManager", m_pThreadManager );
  mainCatalog.Add( "NativeLibraryContainer", m_pNativeLibraryContainer );
  mainCatalog.Add( "ObjectRegistry", m_pObjectRegistry );
  mainCatalog.Add("SearchPaths", m_pFileSearchPathCollection);
  // ************************************************************************************
}

std::shared_ptr<VirtualMachine> VirtualMachine::Create()
{
  std::shared_ptr<VirtualMachine> pThis = std::shared_ptr<VirtualMachine>( new VirtualMachine() );

  pThis->SetupDependencies( pThis );

  return pThis;
}

jint JNICALL VirtualMachine::java_lang_reflect_Constructor_getModifiers( JNIEnv *pEnv, jobject obj )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_reflect_Constructor_getModifiers" );
#endif // _DEBUG

  boost::intrusive_ptr<ObjectReference> pConstructorObject = JNIEnvInternal::ConvertJObjectToObjectPointer( obj );

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  boost::intrusive_ptr<ObjectReference> pClassObject = boost::dynamic_pointer_cast<ObjectReference>( pConstructorObject->GetContainedObject()->GetFieldByName( JavaString::FromCString( "clazz" ) ) );
  boost::intrusive_ptr<JavaInteger> pSlot = boost::dynamic_pointer_cast<JavaInteger>( pConstructorObject->GetContainedObject()->GetFieldByName( JavaString::FromCString( "slot" ) ) );

  if ( nullptr == pClassObject )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected class object in field 'clazz' of current object." );
  }

  if ( nullptr == pSlot )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected integer in field 'slot' of current object." );
  }

  boost::intrusive_ptr<JavaString> pClassName = boost::dynamic_pointer_cast<JavaString>( pClassObject->GetContainedObject()->GetJVMXFieldByName( c_SyntheticField_ClassName ) );
  if ( nullptr == pClassName )
  {
    throw InvalidStateException( __FUNCTION__ " - Error reading class name of Class object." );
  }

  std::shared_ptr<JavaClass> pActualClass = pVirtualMachineState->LoadClass( *pClassName );
  if ( nullptr == pActualClass )
  {
    throw InvalidStateException( __FUNCTION__ " - Could not find the class matching the name of the constructor's class object." );
  }

  if ( !pActualClass->IsInitialsed() )
  {
    pVirtualMachineState->InitialiseClass( *pClassName );
  }

  std::shared_ptr<MethodInfo> pMethodInfo = pActualClass->GetMethodByIndex( pSlot->ToHostInt32() );
  if ( nullptr == pMethodInfo )
  {
    throw InvalidStateException( __FUNCTION__ " - Could not get the method indicated by the relevant slot." );
  }

  return pMethodInfo->GetFlags() & 0x0FFF;
}

jobject JNICALL VirtualMachine::java_lang_reflect_VMConstructor_construct( JNIEnv *pEnv, jobject obj, jarray args )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_reflect_VMConstructor_construct( 0x%p )", args );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  pVirtualMachineState->LogOperandStack();
  pVirtualMachineState->LogCallStack();
#endif // _DEBUG

  boost::intrusive_ptr<ObjectReference> pObject = JNIEnvInternal::ConvertJObjectToObjectPointer( obj );
  boost::intrusive_ptr<ObjectReference> pFieldClazz = boost::dynamic_pointer_cast<ObjectReference>( pObject->GetContainedObject()->GetFieldByName( JavaString::FromCString( u"clazz" ) ) );
  boost::intrusive_ptr<JavaInteger> pFieldSlot = boost::dynamic_pointer_cast<JavaInteger>( pObject->GetContainedObject()->GetFieldByName( JavaString::FromCString( u"slot" ) ) );

  boost::intrusive_ptr<JavaString> pClazzName = boost::dynamic_pointer_cast<JavaString>( pFieldClazz->GetContainedObject()->GetJVMXFieldByName( c_SyntheticField_ClassName ) );

  std::shared_ptr<JavaClass> pClass = pVirtualMachineState->LoadClass( *pClazzName );
  std::shared_ptr<MethodInfo> pMethodInfo = pClass->GetMethodByIndex( pFieldSlot->ToHostInt32() );

  if ( !pClass->IsInitialsed() )
  {
    pVirtualMachineState->InitialiseClass( *pClass->GetName() );
  }

  _jmethodID _methodID = { pMethodInfo };

  std::unique_ptr<jvalue[]> pArgsList = ConvertJArrayToArrayOfObjects( args, TypeParser::ParseMethodType( *pMethodInfo->GetType() ) );

  return pEnv->NewObjectA( pEnv, static_cast< jclass >( pFieldClazz->ToJObject() ), &_methodID, pArgsList.get() );
}

boost::intrusive_ptr<ObjectReference> VirtualMachine::BuildParameterArray( TypeParser::ParsedMethodType &methodParams, JNIEnv *pEnv )
{
  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  boost::intrusive_ptr<ObjectReference> pParameterArray = pVirtualMachineState->CreateArray( e_JavaArrayTypes::Reference, methodParams.parameters.size() );

  uint32_t parameterIndex = 0;

#ifdef _DEBUG
  const size_t debugSize = pVirtualMachineState->GetOperandStackSize();
#endif // _DEBUG

  for ( auto it = methodParams.parameters.begin(); it != methodParams.parameters.end(); ++ it )
  {
    size_t characterIndex = 0;
    char16_t type = ( *it )->At( characterIndex );

    if ( TypeParser::IsPrimitiveTypeDescriptor( type ) )
    {
      boost::intrusive_ptr<ObjectReference> pPrimitiveClassObjectPtr = VirtualMachine::GetPrimitiveClass( ( *it )->At( 0 ), pVirtualMachineState );

      pParameterArray->GetContainedArray()->SetAt( parameterIndex, pPrimitiveClassObjectPtr.get() );
    }
    else if ( c_JavaTypeSpecifierArray == type )
    {
      auto pJavaLangClassInstance = pVirtualMachineState->CreateJavaLangClassFromClassName( *it );
      pParameterArray->GetContainedArray()->SetAt( parameterIndex, pJavaLangClassInstance.get() );

      // Not Implemented yet.
      //JVMX_ASSERT( false );
      //      }
    }
    else if ( c_JavaTypeSpecifierReference == type )
    {
      ++ characterIndex;
      JavaString className = ( *it )->SubString( characterIndex );

      if ( className.EndsWith( JVMX_T( ";" ) ) )
      {
        className = className.SubString( 0, className.GetLengthInCodePoints() - 1 );
      }

      auto pClass = pVirtualMachineState->LoadClass( className );
      auto pJavaLangClassInstance = pVirtualMachineState->CreateJavaLangClassFromClassName( pClass->GetName() );
      pParameterArray->GetContainedArray()->SetAt( parameterIndex, pJavaLangClassInstance.get() );
    }

    ++ parameterIndex;
  }

#ifdef _DEBUG
  JVMX_ASSERT( pVirtualMachineState->GetOperandStackSize() == debugSize );
#endif // _DEBUG

  return pParameterArray;
}

jarray JNICALL VirtualMachine::java_lang_reflect_VMConstructor_getParameterTypes( JNIEnv *pEnv, jobject obj )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_reflect_VMConstructor_getParameterTypes" );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  boost::intrusive_ptr<ObjectReference> pObject = JNIEnvInternal::ConvertJObjectToObjectPointer( obj );
  boost::intrusive_ptr<ObjectReference> pFieldClazz = boost::dynamic_pointer_cast<ObjectReference>( pObject->GetContainedObject()->GetFieldByName( JavaString::FromCString( u"clazz" ) ) );
  boost::intrusive_ptr<JavaInteger> pFieldSlot = boost::dynamic_pointer_cast<JavaInteger>( pObject->GetContainedObject()->GetFieldByName( JavaString::FromCString( u"slot" ) ) );

  boost::intrusive_ptr<JavaString> pClazzName = boost::dynamic_pointer_cast<JavaString>( pFieldClazz->GetContainedObject()->GetJVMXFieldByName( c_SyntheticField_ClassName ) );

  std::shared_ptr<JavaClass> pClazzValue = pVirtualMachineState->LoadClass( *pClazzName );
  std::shared_ptr<MethodInfo> pMethodInfo = pClazzValue->GetMethodByIndex( pFieldSlot->ToHostInt32() );

  TypeParser::ParsedMethodType methodParams = TypeParser::ParseMethodType( *pMethodInfo->GetType() );
  boost::intrusive_ptr<ObjectReference> pParameterArray = BuildParameterArray( methodParams, pEnv );

  return JNIEnvInternal::ConvertArrayPointerToJArray( pVirtualMachineState, pParameterArray );
}

jint JNICALL VirtualMachine::java_lang_reflect_VMConstructor_getModifiersInternal( JNIEnv *pEnv, jobject obj )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_reflect_VMConstructor_getModifiersInternal" );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  boost::intrusive_ptr<ObjectReference> pObject = JNIEnvInternal::ConvertJObjectToObjectPointer( obj );
  boost::intrusive_ptr<ObjectReference> pFieldClazz = boost::dynamic_pointer_cast<ObjectReference>( pObject->GetContainedObject()->GetFieldByName( JavaString::FromCString( u"clazz" ) ) );
  boost::intrusive_ptr<JavaInteger> pFieldSlot = boost::dynamic_pointer_cast<JavaInteger>( pObject->GetContainedObject()->GetFieldByName( JavaString::FromCString( u"slot" ) ) );

  boost::intrusive_ptr<JavaString> pClazzName = boost::dynamic_pointer_cast<JavaString>( pFieldClazz->GetContainedObject()->GetJVMXFieldByName( c_SyntheticField_ClassName ) );

  std::shared_ptr<JavaClass> pClazzValue = pVirtualMachineState->LoadClass( *pClazzName );
  std::shared_ptr<MethodInfo> pMethodInfo = pClazzValue->GetMethodByIndex( pFieldSlot->ToHostInt32() );

  return pMethodInfo->GetFlags();
}

void VirtualMachine::CallThreadConstructor( const std::shared_ptr<IVirtualMachineState> &pInitialState, std::shared_ptr<JavaClass> &pThreadClass, boost::intrusive_ptr<ObjectReference> pThread, boost::intrusive_ptr<ObjectReference> pVMThread )
{
  static const JavaString c_MethodType = JavaString::FromCString( "(Ljava/lang/VMThread;Ljava/lang/String;IZ)V" );
  std::shared_ptr<MethodInfo> pMethodInfo = pInitialState->ResolveMethod( pThreadClass.get(), c_InstanceInitialisationMethodName, c_MethodType );

  if ( pMethodInfo->IsSynchronised() )
  {
    pInitialState->PushMonitor( pThread->GetContainedObject()->MonitorEnter( pInitialState->GetCurrentClassAndMethodName().ToUtf8String().c_str() ) );
  }

  pInitialState->PushOperand( pThread );
  pInitialState->PushOperand( pVMThread );
  pInitialState->PushOperand( pInitialState->CreateStringObject( "main" ) );
  pInitialState->PushOperand( boost::intrusive_ptr<IJavaVariableType>( new JavaInteger( JavaInteger::FromHostInt32( 10 ) ) ) );
  pInitialState->PushOperand( boost::intrusive_ptr<IJavaVariableType>( new JavaBool( JavaBool::FromBool( false ) ) ) );

  pInitialState->ExecuteMethod( *pThreadClass->GetName(), c_InstanceInitialisationMethodName, c_MethodType, pMethodInfo );

  // Shouldn't there be a PopMonitor here?
}

void VirtualMachine::CallVMThreadConstructor( const std::shared_ptr<IVirtualMachineState> &pInitialState, std::shared_ptr<JavaClass> &pVMThreadClass, boost::intrusive_ptr<ObjectReference> pThread, boost::intrusive_ptr<ObjectReference> pVMThread )
{
  static const JavaString c_MethodType = JavaString::FromCString( "(Ljava/lang/Thread;)V" );
  std::shared_ptr<MethodInfo> pMethodInfo = pInitialState->ResolveMethod( pVMThreadClass.get(), c_InstanceInitialisationMethodName, c_MethodType );

  if ( pMethodInfo->IsSynchronised() )
  {
    pInitialState->PushMonitor( pThread->GetContainedObject()->MonitorEnter( pInitialState->GetCurrentClassAndMethodName().ToUtf8String().c_str() ) );
  }

  pInitialState->PushOperand( pVMThread );
  pInitialState->PushOperand( pThread );
  pInitialState->ExecuteMethod( *pVMThreadClass->GetName(), c_InstanceInitialisationMethodName, c_MethodType, pMethodInfo );

  // Shouldn't there be a PopMonitor here?
}

void VirtualMachine::AddThreadToGroup( const std::shared_ptr<IVirtualMachineState> &pInitialState, std::shared_ptr<JavaClass> &pThreadGroupClass, boost::intrusive_ptr<ObjectReference> pRootGroup, boost::intrusive_ptr<ObjectReference> pThread )
{
  std::shared_ptr<MethodInfo> pMethodInfo = pInitialState->ResolveMethod( pThreadGroupClass.get(), JavaString::FromCString( "addThread" ), JavaString::FromCString( "(Ljava/lang/Thread;)V" ) );

  pInitialState->PushOperand( pRootGroup );
  pInitialState->PushOperand( pThread );
  pInitialState->ExecuteMethod( *pThreadGroupClass->GetName(), JavaString::FromCString( "addThread" ), JavaString::FromCString( "(Ljava/lang/Thread;)V" ), pMethodInfo );
}

void VirtualMachine::AddToInheritableThreadLocal( const std::shared_ptr<IVirtualMachineState> &pInitialState, boost::intrusive_ptr<ObjectReference> pThread )
{
  std::shared_ptr<JavaClass> pThredLocalClass = pInitialState->LoadClass( JavaString::FromCString( JVMX_T( "java/lang/InheritableThreadLocal" ) ) );
  std::shared_ptr<MethodInfo> pMethodInfo = pInitialState->ResolveMethod( pThredLocalClass.get(), JavaString::FromCString( "newChildThread" ), JavaString::FromCString( "(Ljava/lang/Thread;)V" ) );

  if ( pMethodInfo->IsSynchronised() )
  {
    pInitialState->PushMonitor( pThredLocalClass->MonitorEnter( pInitialState->GetCurrentClassAndMethodName().ToUtf8String().c_str() ) );
  }

  pInitialState->PushOperand( pThread );
  pInitialState->ExecuteMethod( *pThredLocalClass->GetName(), JavaString::FromCString( "newChildThread" ), JavaString::FromCString( "(Ljava/lang/Thread;)V" ), pMethodInfo );
}

jarray JNICALL VirtualMachine::gnu_classpath_VMStackWalker_getClassContext( JNIEnv *pEnv, jobject obj )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: gnu_classpath_VMStackWalker_getClassContext" );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  boost::intrusive_ptr<ObjectReference> pResult = pVirtualMachineState->GetCallStackOfClassObjects();

  return JNIEnvInternal::ConvertArrayPointerToJArray( pVirtualMachineState, pResult );
}

jobject JNICALL VirtualMachine::gnu_classpath_VMStackWalker_getClassLoader( JNIEnv *pEnv, jobject obj, jclass clazz )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: gnu_classpath_VMStackWalker_getClassLoader" );
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

jboolean JNICALL VirtualMachine::java_io_VMFile_exists( JNIEnv *pEnv, jobject obj, jstring path )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_io_VMFile_exists( 0x%p )", path );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  jboolean result = JNI_FALSE;
  jboolean isCopy = false;
  const jbyte *bytes = pEnv->GetStringUTFChars( pEnv, path, &isCopy );

  try
  {
    struct _stat buffer = { 0 };
    if ( 0 == _stat( reinterpret_cast<const char *>( bytes ), &buffer ) )
    {
      result = JNI_TRUE;
    }
  }
  catch ( ... )
  {
    pEnv->ReleaseStringUTFChars( pEnv, path, reinterpret_cast<const char *>( bytes ) );
    throw;
  }

  pEnv->ReleaseStringUTFChars( pEnv, path, reinterpret_cast<const char *>( bytes ) );

  return result;
}

jobject JNICALL VirtualMachine::java_lang_VMThrowable_fillInStackTrace( JNIEnv *pEnv, jobject obj, jobject throwAble )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMThrowable_fillInStackTrace(%p)", throwAble );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  boost::intrusive_ptr<ObjectReference> pArrayOfStackTraceElements = pVirtualMachineState->GetCallStackOfStackTraceElements();
  if ( nullptr == pArrayOfStackTraceElements )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected valid array of StaceTraceElement objects." );
  }

  std::shared_ptr<JavaClass> pClassOfVMThrowable = pVirtualMachineState->LoadClass( JavaString::FromCString( "java/lang/VMThrowable" ) );
  if ( nullptr == pClassOfVMThrowable )
  {
    throw InvalidStateException( __FUNCTION__ " - Could not load class VMThrowable." );
  }

  if ( !pClassOfVMThrowable->IsInitialsed() )
  {
    pVirtualMachineState->InitialiseClass( *pClassOfVMThrowable->GetName() );
  }

  boost::intrusive_ptr<ObjectReference> pVMThrowable = pVirtualMachineState->CreateObject( pClassOfVMThrowable );

  // TODO: This is not correct. As per the GNU Classpath documentation, this class needs to be customised for the VM
  // For now though, we simply store this in VMData to simulate the correct execution of this method.
  pVMThrowable->GetContainedObject()->SetField( JavaString::FromCString( "vmdata" ), pArrayOfStackTraceElements );

  return JNIEnvInternal::ConvertObjectPointerToJObject( pVirtualMachineState, pVMThrowable );
}

jobject JNICALL VirtualMachine::Java_java_lang_VMThrowable_getStackTrace( JNIEnv *pEnv, jobject obj, jobject throwAble )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: Java_java_lang_VMThrowable_getStackTrace(%p)", throwAble );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  boost::intrusive_ptr<ObjectReference> pVMThrowable = JNIEnvInternal::ConvertJObjectToObjectPointer( obj );
  if ( nullptr == pVMThrowable )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected object pointer to be non-null." );
  }

  boost::intrusive_ptr<ObjectReference> pArrayOfStackTraceElements = boost::dynamic_pointer_cast<ObjectReference>( pVMThrowable->GetContainedObject()->GetFieldByName( JavaString::FromCString( "vmdata" ) ) );
  if ( nullptr == pArrayOfStackTraceElements )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected valid array of StaceTraceElement objects." );
  }

  return JNIEnvInternal::ConvertObjectPointerToJObject( pVirtualMachineState, pArrayOfStackTraceElements );
}

jdouble JNICALL VirtualMachine::java_lang_VMMath_log( JNIEnv *pEnv, jobject obj, jdouble value )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMMath_log(%f)", value );
#endif // _DEBUG

  return log( value );
}

jdouble JNICALL VirtualMachine::java_lang_VMMath_exp( JNIEnv *pEnv, jobject obj, jdouble value )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMMath_log(%f)", value );
#endif // _DEBUG

  return exp( value );
}

jobject JNICALL VirtualMachine::java_lang_reflect_VMField_get( JNIEnv *pEnv, jobject obj, jobject objectToGetValueFrom )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_reflect_VMField_get(%d)", objectToGetValueFrom );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  boost::intrusive_ptr<ObjectReference> pVMField = JNIEnvInternal::ConvertJObjectToObjectPointer( obj );
  boost::intrusive_ptr<ObjectReference> pObject = JNIEnvInternal::ConvertJObjectToObjectPointer( objectToGetValueFrom );

  boost::intrusive_ptr<ObjectReference> pFieldName = boost::dynamic_pointer_cast<ObjectReference>( pVMField->GetContainedObject()->GetFieldByNameConst( JavaString::FromCString( u"name" ) ) );

  boost::intrusive_ptr<IJavaVariableType> pNameObject = pFieldName->GetContainedObject()->GetFieldByName( JavaString::FromCString( u"value" ) );

  boost::intrusive_ptr<ObjectReference> pName = boost::dynamic_pointer_cast<ObjectReference>( pNameObject );
  JavaString nameAsString = pName->GetContainedArray()->ConvertCharArrayToString();

  boost::intrusive_ptr<IJavaVariableType> pValue = nullptr;
  if ( pObject->IsNull() )
  {
    boost::intrusive_ptr<ObjectReference> pFieldClass = boost::dynamic_pointer_cast<ObjectReference>( pVMField->GetContainedObject()->GetFieldByNameConst( JavaString::FromCString( u"clazz" ) ) );
    boost::intrusive_ptr<JavaString> pClassName = boost::dynamic_pointer_cast<JavaString>( pFieldClass->GetContainedObject()->GetJVMXFieldByName( c_SyntheticField_ClassName ) );

    std::shared_ptr<JavaClass> pClass = pVirtualMachineState->LoadClass( *pClassName );
    if ( !pClass->IsInitialsed() )
    {
      pVirtualMachineState->InitialiseClass( *pClassName );
    }

    std::shared_ptr<FieldInfo> pFieldInfo = pClass->GetFieldByName( nameAsString );
    JVMX_ASSERT( pFieldInfo->IsStatic() );

    pValue = pFieldInfo->GetStaticValue();
  }
  else
  {
    pValue = pObject->GetContainedObject()->GetFieldByNameConst( nameAsString );
  }

  switch ( pValue->GetVariableType() )
  {
    case e_JavaVariableTypes::Char:
      {
        static const JavaString c_ResultClassName = JavaString::FromCString( u"java/lang/Character" );

        pVirtualMachineState->InitialiseClass( c_ResultClassName );
        auto pResultClass = pVirtualMachineState->LoadClass( c_ResultClassName );
        auto pResult = pVirtualMachineState->CreateObject( pResultClass );

        auto pMethodInfo = pResultClass->GetMethodByNameAndType( JavaString::FromCString( u"<init>" ), JavaString::FromCString( u"(C)V" ) );

        pVirtualMachineState->PushOperand( pResult );
        pVirtualMachineState->PushOperand( pValue );
        pVirtualMachineState->ExecuteMethod( c_ResultClassName, JavaString::FromCString( u"<init>" ), JavaString::FromCString( u"(C)V" ), pMethodInfo );

        return JNIEnvInternal::ConvertObjectPointerToJObject( pVirtualMachineState, pResult );
      }
      break;

    case e_JavaVariableTypes::Byte:
      {
        static const JavaString c_ResultClassName = JavaString::FromCString( u"java/lang/Byte" );

        pVirtualMachineState->InitialiseClass( c_ResultClassName );
        auto pResultClass = pVirtualMachineState->LoadClass( c_ResultClassName );
        auto pResult = pVirtualMachineState->CreateObject( pResultClass );

        auto pMethodInfo = pResultClass->GetMethodByNameAndType( JavaString::FromCString( u"<init>" ), JavaString::FromCString( u"(B)V" ) );

        pVirtualMachineState->PushOperand( pResult );
        pVirtualMachineState->PushOperand( pValue );
        pVirtualMachineState->ExecuteMethod( c_ResultClassName, JavaString::FromCString( u"<init>" ), JavaString::FromCString( u"(B)V" ), pMethodInfo );

        return JNIEnvInternal::ConvertObjectPointerToJObject( pVirtualMachineState, pResult );
      }
      break;

    case e_JavaVariableTypes::Short:
      {
        static const JavaString c_ResultClassName = JavaString::FromCString( u"java/lang/Short" );

        pVirtualMachineState->InitialiseClass( c_ResultClassName );
        auto pResultClass = pVirtualMachineState->LoadClass( c_ResultClassName );
        auto pResult = pVirtualMachineState->CreateObject( pResultClass );

        auto pMethodInfo = pResultClass->GetMethodByNameAndType( JavaString::FromCString( u"<init>" ), JavaString::FromCString( u"(S)V" ) );

        pVirtualMachineState->PushOperand( pResult );
        pVirtualMachineState->PushOperand( pValue );
        pVirtualMachineState->ExecuteMethod( c_ResultClassName, JavaString::FromCString( u"<init>" ), JavaString::FromCString( u"(S)V" ), pMethodInfo );

        return JNIEnvInternal::ConvertObjectPointerToJObject( pVirtualMachineState, pResult );
      }
      break;
    case e_JavaVariableTypes::Integer:
      {
        static const JavaString c_ResultClassName = JavaString::FromCString( u"java/lang/Integer" );

        pVirtualMachineState->InitialiseClass( c_ResultClassName );
        auto pResultClass = pVirtualMachineState->LoadClass( c_ResultClassName );
        auto pResult = pVirtualMachineState->CreateObject( pResultClass );

        auto pMethodInfo = pResultClass->GetMethodByNameAndType( JavaString::FromCString( u"<init>" ), JavaString::FromCString( u"(I)V" ) );

        pVirtualMachineState->PushOperand( pResult );
        pVirtualMachineState->PushOperand( pValue );
        pVirtualMachineState->ExecuteMethod( c_ResultClassName, JavaString::FromCString( u"<init>" ), JavaString::FromCString( u"(I)V" ), pMethodInfo );

        return JNIEnvInternal::ConvertObjectPointerToJObject( pVirtualMachineState, pResult );
      }
      break;

    case e_JavaVariableTypes::Long:
      {
        static const JavaString c_ResultClassName = JavaString::FromCString( u"java/lang/Long" );

        pVirtualMachineState->InitialiseClass( c_ResultClassName );
        auto pResultClass = pVirtualMachineState->LoadClass( c_ResultClassName );
        auto pResult = pVirtualMachineState->CreateObject( pResultClass );

        auto pMethodInfo = pResultClass->GetMethodByNameAndType( JavaString::FromCString( u"<init>" ), JavaString::FromCString( u"(J)V" ) );

        pVirtualMachineState->PushOperand( pResult );
        pVirtualMachineState->PushOperand( pValue );
        pVirtualMachineState->ExecuteMethod( c_ResultClassName, JavaString::FromCString( u"<init>" ), JavaString::FromCString( u"(J)V" ), pMethodInfo );

        return JNIEnvInternal::ConvertObjectPointerToJObject( pVirtualMachineState, pResult );
      }
      break;

    case e_JavaVariableTypes::Float:
      {
        static const JavaString c_ResultClassName = JavaString::FromCString( u"java/lang/Float" );

        pVirtualMachineState->InitialiseClass( c_ResultClassName );
        auto pResultClass = pVirtualMachineState->LoadClass( c_ResultClassName );
        auto pResult = pVirtualMachineState->CreateObject( pResultClass );

        auto pMethodInfo = pResultClass->GetMethodByNameAndType( JavaString::FromCString( u"<init>" ), JavaString::FromCString( u"(F)V" ) );

        pVirtualMachineState->PushOperand( pResult );
        pVirtualMachineState->PushOperand( pValue );
        pVirtualMachineState->ExecuteMethod( c_ResultClassName, JavaString::FromCString( u"<init>" ), JavaString::FromCString( u"(F)V" ), pMethodInfo );

        return JNIEnvInternal::ConvertObjectPointerToJObject( pVirtualMachineState, pResult );
      }
      break;
    case e_JavaVariableTypes::Double:
      {
        static const JavaString c_ResultClassName = JavaString::FromCString( u"java/lang/Double" );

        pVirtualMachineState->InitialiseClass( c_ResultClassName );
        auto pResultClass = pVirtualMachineState->LoadClass( c_ResultClassName );
        auto pResult = pVirtualMachineState->CreateObject( pResultClass );

        auto pMethodInfo = pResultClass->GetMethodByNameAndType( JavaString::FromCString( u"<init>" ), JavaString::FromCString( u"(D)V" ) );

        pVirtualMachineState->PushOperand( pResult );
        pVirtualMachineState->PushOperand( pValue );
        pVirtualMachineState->ExecuteMethod( c_ResultClassName, JavaString::FromCString( u"<init>" ), JavaString::FromCString( u"(D)V" ), pMethodInfo );

        return JNIEnvInternal::ConvertObjectPointerToJObject( pVirtualMachineState, pResult );
      }
      break;

    case e_JavaVariableTypes::String:
      return JNIEnvInternal::ConvertObjectPointerToJObject( pVirtualMachineState, pVirtualMachineState->CreateStringObject( *boost::dynamic_pointer_cast<JavaString>( pValue ) ) );
      break;

    case e_JavaVariableTypes::Bool:
      {
        static const JavaString c_ResultClassName = JavaString::FromCString( u"java/lang/Boolean" );

        pVirtualMachineState->InitialiseClass( c_ResultClassName );
        auto pResultClass = pVirtualMachineState->LoadClass( c_ResultClassName );
        auto pResult = pVirtualMachineState->CreateObject( pResultClass );

        auto pMethodInfo = pResultClass->GetMethodByNameAndType( JavaString::FromCString( u"<init>" ), JavaString::FromCString( u"(Z)V" ) );

        pVirtualMachineState->PushOperand( pResult );
        pVirtualMachineState->PushOperand( pValue );
        pVirtualMachineState->ExecuteMethod( c_ResultClassName, JavaString::FromCString( u"<init>" ), JavaString::FromCString( u"(Z)V" ), pMethodInfo );

        return JNIEnvInternal::ConvertObjectPointerToJObject( pVirtualMachineState, pResult );
      }
      break;

    case e_JavaVariableTypes::NullReference:
      return JNIEnvInternal::ConvertNullPointerToJObject();
      break;

    case e_JavaVariableTypes::Array:
    case e_JavaVariableTypes::Object:
      return JNIEnvInternal::ConvertObjectPointerToJObject( pVirtualMachineState, boost::dynamic_pointer_cast<ObjectReference>( pValue ) );
      break;

    default:
      throw InvalidStateException( __FUNCTION__ " - Unexpected type." );
  }

  return JNIEnvInternal::ConvertNullPointerToJObject();
}

jint JNICALL VirtualMachine::java_lang_reflect_VMField_getModifiersInternal( JNIEnv *pEnv, jobject obj )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_reflect_VMField_getModifiersInternal(%d)", obj);
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  boost::intrusive_ptr<ObjectReference> pVMField = JNIEnvInternal::ConvertJObjectToObjectPointer( obj );

  boost::intrusive_ptr<ObjectReference> pJavaLangClass = boost::dynamic_pointer_cast<ObjectReference>( pVMField->GetContainedObject()->GetFieldByName( JavaString::FromCString( JVMX_T( "clazz" ) ) ) );
  if ( nullptr == pJavaLangClass )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected class object in field 'clazz' of current object." );
  }

  boost::intrusive_ptr<JavaString> pClassName = boost::dynamic_pointer_cast<JavaString>( pJavaLangClass->GetContainedObject()->GetJVMXFieldByName( c_SyntheticField_ClassName ) );
  if ( nullptr == pClassName )
  {
    throw InvalidStateException( __FUNCTION__ " - Error reading class name of Class object." );
  }

  std::shared_ptr<JavaClass> pClass = pVirtualMachineState->LoadClass( *pClassName );
  if ( nullptr == pClass )
  {
    throw InvalidStateException( __FUNCTION__ " - Could not find class." );
  }

  boost::intrusive_ptr<ObjectReference> pFieldNameObject = boost::dynamic_pointer_cast<ObjectReference>( pVMField->GetContainedObject()->GetFieldByName( JavaString::FromCString( JVMX_T( "name" ) ) ) );
  if ( nullptr == pFieldNameObject )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected String object in field 'name' of current object." );
  }

  JavaString fieldName = HelperTypes::ExtractValueFromStringObject( pFieldNameObject );

  std::shared_ptr<FieldInfo> pFieldInfo = pClass->GetFieldByName( fieldName );
  if ( nullptr == pFieldInfo )
  {
    throw InvalidStateException( __FUNCTION__ " - Could not find the field on the class object." );
  }

  return pFieldInfo->GetFlags();
}

jclass VirtualMachine::java_lang_reflect_VMField_getType( struct JNIEnv *pEnv, jobject obj )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_reflect_VMField_getType(%d)", obj);
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  boost::intrusive_ptr<ObjectReference> pVMField = JNIEnvInternal::ConvertJObjectToObjectPointer( obj );

  boost::intrusive_ptr<ObjectReference> pJavaLangClass = boost::dynamic_pointer_cast<ObjectReference>( pVMField->GetContainedObject()->GetFieldByName( JavaString::FromCString( JVMX_T( "clazz" ) ) ) );
  if ( nullptr == pJavaLangClass )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected class object in field 'clazz' of current object." );
  }

  return static_cast< jclass >( JNIEnvInternal::ConvertObjectPointerToJObject( pVirtualMachineState, pJavaLangClass ) );
}

void JvmxAssert( bool cond, const char *description )
{
#ifdef _DEBUG
  if ( !cond )
  {
    throw AssertionFailedException( description );
  }
#endif // _DEBUG
}