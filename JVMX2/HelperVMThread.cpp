#include <thread>
#include <chrono>
#include <memory>

#include <boost/intrusive_ptr.hpp>

#include "GlobalCatalog.h"
#include "NotImplementedException.h"

#include "BasicVirtualMachineState.h"
#include "JavaNativeInterface.h"
#include "VirtualMachine.h"
#include "JavaClass.h"
#include "ObjectReference.h"
#include "OsFunctions.h"
#include "HelperTypes.h"


#include "HelperVMThread.h"


static void NewThreadFunction( const std::shared_ptr<IVirtualMachineState> &pVMState, boost::intrusive_ptr<ObjectReference> pObject )
{
#ifdef _DEBUG
  boost::intrusive_ptr<ObjectReference> pThreadObject = boost::dynamic_pointer_cast<ObjectReference>( pObject->GetContainedObject()->GetFieldByName( JavaString::FromCString( u"thread" ) ) );
  boost::intrusive_ptr<ObjectReference> pThreadName = boost::dynamic_pointer_cast<ObjectReference>( pThreadObject->GetContainedObject()->GetFieldByName( JavaString::FromCString( u"name" ) ) );


  JavaString threadName = HelperTypes::ExtractValueFromStringObject(pThreadName);
  //boost::intrusive_ptr<IJavaVariableType> pNameAsVariableType = pThreadName->GetContainedObject()->GetFieldByName( JavaString::FromCString( u"value" ) );
  //boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>( pNameAsVariableType );

  //JavaString threadName = pArray->GetContainedArray()->ConvertCharArrayToString();
  OsFunctions::GetInstance().SetThreadName( threadName.ToUtf8String().c_str() );
#endif // _DEBUG

  std::shared_ptr<MethodInfo> pMethodInfo = pVMState->ResolveMethod( pObject->GetContainedObject()->GetClass().get(), JavaString::FromCString( u"run" ), JavaString::FromCString( u"()V" ) );

  std::shared_ptr<JavaNativeInterface> pJNI = std::make_shared<JavaNativeInterface>();
  pVMState->SetJavaNativeInterface( pJNI );
  pJNI->SetVMState( pVMState );

  pVMState->RegisterNativeMethods( pJNI );

  pVMState->PushOperand( pObject ); // This object.
  pVMState->ExecuteMethod( *pObject->GetContainedObject()->GetClass()->GetName(), JavaString::FromCString( u"run" ), JavaString::FromCString( u"()V" ), pMethodInfo );
}

void JNICALL HelperVMThread::java_lang_VMThread_sleep( JNIEnv *pEnv, jobject obj, jlong ms, jint ns )
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: Java_java_lang_VMThread_sleep(%lld, %ld)\n", ms, ns );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  std::this_thread::sleep_for( std::chrono::milliseconds( ms ) + std::chrono::nanoseconds( ns ) );
}

void JNICALL HelperVMThread::java_lang_VMThread_yield( JNIEnv *pEnv, jobject obj )
{
#ifdef _DEBUG
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMThread_yield()\n" );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  std::this_thread::yield();
}

void JNICALL HelperVMThread::java_lang_VMThread_suspend( JNIEnv *pEnv, jobject obj )
{
#ifdef _DEBUG
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMThread_suspend()\n" );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  std::thread::native_handle_type threadHandle = pVirtualMachineState->ReturnCurrentThreadInfo().m_pThread->native_handle();
  OsFunctions::GetInstance().SuspendThread( threadHandle );
}

void JNICALL HelperVMThread::java_lang_VMThread_resume( JNIEnv *pEnv, jobject obj )
{
#ifdef _DEBUG
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMThread_resume()\n" );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  std::thread::native_handle_type threadHandle = pVirtualMachineState->ReturnCurrentThreadInfo().m_pThread->native_handle();
  OsFunctions::GetInstance().ResumeThread( threadHandle );
}
void JNICALL HelperVMThread::java_lang_VMThread_interrupt( JNIEnv *pEnv, jobject obj )
{
#ifdef _DEBUG
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMThread_interrupt()\n" );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

  pVirtualMachineState->Interrupt();
}


jboolean JNICALL HelperVMThread::java_lang_VMThread_interrupted( JNIEnv *pEnv, jobject obj )
{
#ifdef _DEBUG
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMThread_interrupted()\n" );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

#ifdef _DEBUG
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      pVirtualMachineState->LogLocalVariables();
      pVirtualMachineState->LogCallStack();
  }
#endif // _DEBUG

  return pVirtualMachineState->GetInterruptedFlag() ? JNI_TRUE : JNI_FALSE;
}

jobject JNICALL HelperVMThread::java_lang_VMThread_currentThread( JNIEnv *pEnv, jobject obj )
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMThread_currentThread()\n" );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      pVirtualMachineState->LogCallStack();
      pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG


  boost::intrusive_ptr<ObjectReference> pCurrentThreadObject = pVirtualMachineState->ReturnCurrentThreadInfo().m_pThreadObject;

  if ( nullptr == pCurrentThreadObject )
  {
    return JNIEnvInternal::ConvertNullPointerToJObject();
  }

  return JNIEnvInternal::ConvertObjectPointerToJObject( pVirtualMachineState, pCurrentThreadObject );
}

void JNICALL HelperVMThread::java_lang_VMThread_start( JNIEnv *pEnv, jobject obj, jlong stackSize )
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMThread_start(%lld)\n", stackSize );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pOldVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pOldVirtualMachineState->HasUserCodeStarted())
  {
      pOldVirtualMachineState->LogOperandStack();
  }
#endif

  const std::shared_ptr<IVirtualMachineState> &pNewVirtualMachineState = pOldVirtualMachineState->CreateNewState();

  boost::intrusive_ptr<ObjectReference> pVMThreadObject = JNIEnvInternal::ConvertJObjectToObjectPointer( obj );
  boost::intrusive_ptr<ObjectReference> pThreadObject = boost::dynamic_pointer_cast<ObjectReference>( pVMThreadObject->GetContainedObject()->GetFieldByName( JavaString::FromCString( u"thread" ) ) );

  std::shared_ptr<boost::thread> pNewThread = std::make_shared<boost::thread>( &NewThreadFunction, pNewVirtualMachineState, pVMThreadObject );

  pOldVirtualMachineState->AddThread( pNewThread, pThreadObject, pNewVirtualMachineState );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pOldVirtualMachineState->HasUserCodeStarted())
  {
      pOldVirtualMachineState->LogOperandStack();
  }
#endif
}