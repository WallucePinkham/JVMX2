#ifndef _VIRTUALMACHINE__H_
#define _VIRTUALMACHINE__H_

//#include <map>
#include <memory>
#include <thread>

//#include <wallaroo/part.h>
//#include <wallaroo/catalog.h>

#include <boost/program_options/variables_map.hpp>
#include <boost/intrusive_ptr.hpp>


#include "GlobalConstants.h"
#include "JavaTypes.h"
#include "include/jni.h"
#include "ThreadManager.h"
#include "TypeParser.h"
#include "NativeLibraryContainer.h"

// Forward Declarations
class IMemoryManager;
class IStackManager;
class ILogger;
class IVirtualMachineState;
class IClassLibrary;
class JavaObject;
class JavaNativeInterface;
class IJavaVariableType;
class IGarbageCollector;
class IExecutionEngine;
class IJavaLangClassList;
class FileSearchPathCollection;

class VirtualMachine : public std::enable_shared_from_this<VirtualMachine>
{
protected:
  VirtualMachine();

public:
  static std::shared_ptr<VirtualMachine> Create();

  void Initialise(const std::string& startingClassfile, const std::shared_ptr<IVirtualMachineState> &pInitialState );
  void Run( const JavaString &fileName, const std::shared_ptr<IVirtualMachineState> &pInitialState, bool userCode = true );
  void Stop( const std::shared_ptr<IVirtualMachineState> &pInitialState );

  std::shared_ptr<JavaNativeInterface> GetNativeInterface() const;

  // jint JNI_CreateJavaVM( JavaVM **pvm, void **penv, void *vm_args ) { throw "Not Implemented yet." };
  // jint JNI_GetCreatedJavaVMs( JavaVM **vmBuf, jsize bufLen, jsize *nVMs );
  // jint JNI_GetDefaultJavaVMInitArgs(void *vm_args);
  // jint JNI_OnLoad(JavaVM *vm, void *reserved);
  // void JNI_OnUnload(JavaVM *vm, void *reserved);

  ThreadInfo ReturnCurrentThreadObject() const;
  void AddThread( std::shared_ptr<boost::thread> pNewThread, boost::intrusive_ptr<ObjectReference> pObject, const std::shared_ptr<IVirtualMachineState> &pNewState );

  // Static Methods
public:
  static boost::intrusive_ptr<ObjectReference> GetPrimitiveClass( jstring str, IVirtualMachineState *pVirtualMachineState );
  static boost::intrusive_ptr<ObjectReference> GetPrimitiveClass( jchar chr, IVirtualMachineState *pVirtualMachineState );

  static const char *GetClassNameFromType( JavaString finalStringValue );

  static JavaString GetClassNameFromFileName( const JavaString &fileName );

  void RegisterNativeMethods( std::shared_ptr<JavaNativeInterface> pJNI );

private:
  void LoadFile( const JVMX_CHAR_TYPE *pFileName );

  void SetupDependencies( std::shared_ptr<VirtualMachine> pThis );

  void InitialiseClass( const JVMX_CHAR_TYPE *pClassName, const std::shared_ptr<IVirtualMachineState> &pInitialState );

  int GetMainClassFromJarFile(const JavaString& fileName, DataBuffer& mainClassOuput);

private:
  static jobject JNICALL java_lang_VMClassLoader_getPrimitiveClass( JNIEnv *pEnv, jobject obj, jchar typeAsChar );

  static jobject JNICALL java_lang_VMSecurityManager_currentClassLoader( JNIEnv *pEnv, jobject obj );

  static jobject JNICALL java_lang_VMObject_clone( JNIEnv *pEnv, jobject obj, jobject objectToClone );
  static void JNICALL java_lang_VMObject_wait( JNIEnv *pEnv, jobject obj, jobject objectToWaitFor, jlong ms, jint ns );
  static jobject JNICALL java_lang_VMObject_getClass( JNIEnv *pEnv, jobject obj, jobject objectToGetClassFor );
  static void JNICALL java_lang_VMObject_notifyAll( JNIEnv *pEnv, jobject obj, jobject objectToNotifyOn );
  static void JNICALL java_lang_VMObject_notify( JNIEnv *pEnv, jobject obj, jobject objectToNotifyOn );

  //static jobject JNICALL java_lang_reflect_Constructor_constructNative( JNIEnv *pEnv, jobject obj, jarray args, jclass declaringClass, jint slot );
  static jint JNICALL java_lang_reflect_Constructor_getModifiers( JNIEnv *pEnv, jobject obj );
  static jobject JNICALL java_lang_reflect_VMConstructor_construct( JNIEnv *pEnv, jobject obj, jarray args );

  static jarray JNICALL java_lang_reflect_VMConstructor_getParameterTypes( JNIEnv *pEnv, jobject obj );
  static jint JNICALL java_lang_reflect_VMConstructor_getModifiersInternal( JNIEnv *pEnv, jobject obj );

  static jboolean JNICALL java_io_VMFile_isDirectory( JNIEnv *pEnv, jobject obj, jstring path );
  static jboolean JNICALL java_io_VMFile_exists( JNIEnv *pEnv, jobject obj, jstring path );


  static jarray JNICALL gnu_classpath_VMStackWalker_getClassContext( JNIEnv *pEnv, jobject obj );
  static jobject JNICALL gnu_classpath_VMStackWalker_getClassLoader( JNIEnv *pEnv, jobject obj, jclass clazz );

  static jobject JNICALL java_lang_VMThrowable_fillInStackTrace( JNIEnv *pEnv, jobject obj, jobject throwAble );
  static jobject JNICALL Java_java_lang_VMThrowable_getStackTrace( JNIEnv *pEnv, jobject obj, jobject throwAble );
  static jdouble JNICALL java_lang_VMMath_log( JNIEnv *pEnv, jobject obj, jdouble value );
  static jdouble JNICALL java_lang_VMMath_exp( JNIEnv *pEnv, jobject obj, jdouble value );

  static jobject JNICALL java_lang_reflect_VMField_get( JNIEnv *pEnv, jobject obj, jobject objectToGetValueFrom );
  static jint JNICALL java_lang_reflect_VMField_getModifiersInternal( JNIEnv *pEnv, jobject obj );
  static jclass JNICALL java_lang_reflect_VMField_getType( JNIEnv *pEnv, jobject obj );

private:
  static std::unique_ptr<jvalue[]> ConvertJArrayToArrayOfObjects( jarray args, TypeParser::ParsedMethodType methodType );
  static jvalue ConvertJavaVariableToJValue( const boost::intrusive_ptr<IJavaVariableType> pElementValue, boost::intrusive_ptr<JavaString> parameterType );

  void CreateInitialThreadObject( const std::shared_ptr<IVirtualMachineState> &pInitialState );
  void AddToInheritableThreadLocal( const std::shared_ptr<IVirtualMachineState> &pInitialState, boost::intrusive_ptr<ObjectReference> pThread );
  void AddThreadToGroup( const std::shared_ptr<IVirtualMachineState> &pInitialState, std::shared_ptr<JavaClass> &pThreadGroupClass, boost::intrusive_ptr<ObjectReference> pRootGroup, boost::intrusive_ptr<ObjectReference> pThread );
  void CallThreadConstructor( const std::shared_ptr<IVirtualMachineState> &pInitialState, std::shared_ptr<JavaClass> &pThreadClass, boost::intrusive_ptr<ObjectReference> pThread, boost::intrusive_ptr<ObjectReference> pVMThread );
  void CallVMThreadConstructor( const std::shared_ptr<IVirtualMachineState> &pInitialState, std::shared_ptr<JavaClass> &pVMThreadClass, boost::intrusive_ptr<ObjectReference> pThread, boost::intrusive_ptr<ObjectReference> pVMThread );

  static boost::intrusive_ptr<ObjectReference> BuildParameterArray( TypeParser::ParsedMethodType &methodParams, JNIEnv *pEnv );

private:
  std::shared_ptr<ILogger> m_pLogger;
  std::shared_ptr<IGarbageCollector> m_pGarbageCollector;
  std::shared_ptr<IClassLibrary> m_pRuntimeConstantPool;
  std::shared_ptr<IExecutionEngine> m_pEngine;
  std::shared_ptr<JavaNativeInterface> m_pJNI;
  std::shared_ptr<IJavaLangClassList> m_pJavaLangClassList;
  std::shared_ptr<IThreadManager> m_pThreadManager;
  std::shared_ptr<NativeLibraryContainer> m_pNativeLibraryContainer;
  std::shared_ptr<IObjectRegistry> m_pObjectRegistry;
  std::shared_ptr<FileSearchPathCollection> m_pFileSearchPathCollection;
};

#endif // _VIRTUALMACHINE__H_
