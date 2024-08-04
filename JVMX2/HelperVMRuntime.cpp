#include "JavaNativeInterface.h"
#include "GlobalCatalog.h"

#include "ILogger.h"
#include "IVirtualMachineState.h"
#include "IClassLibrary.h"

#include "ObjectReference.h"

#include "OsFunctions.h"

#include "HelperTypes.h"
#include "HelperVMRuntime.h"

void JNICALL HelperVMRuntime::java_lang_VMRuntime_insertSystemProperties( JNIEnv *pEnv, jobject obj, jobject properties )
{
}

jstring JNICALL HelperVMRuntime::Java_java_lang_VMRuntime_mapLibraryName( JNIEnv *pEnv, jobject obj, jstring libName )
{
  boost::intrusive_ptr<ObjectReference> pLibName = JNIEnvInternal::ConvertJObjectToObjectPointer( libName );

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: Java_java_lang_VMRuntime_mapLibraryName(%s)\n", reinterpret_cast<const char *>(pLibName->ToString().ToUtf8String().c_str()) );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>(pEnv);
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>(pInternal->m_pInternal);

  JavaString nameAsString = HelperTypes::ExtractValueFromStringObject( pLibName );
  if ( nameAsString.Equals( JVMX_T("javalang" ) ))
  {
    return pEnv->NewStringUTF( pEnv, "javalang.dll" );
  }

  if ( nameAsString.Equals( JVMX_T("javanio" ) ))
  {
    return pEnv->NewStringUTF( pEnv, "javalang.dll" );
  }

  if ( nameAsString.Equals( JVMX_T("javaio" ) ))
  {
    return pEnv->NewStringUTF( pEnv, "javalang.dll" );
  }

  std::string mappedName = OsFunctions::GetInstance().MapLibraryName( reinterpret_cast<const char *>(nameAsString.ToUtf8String().c_str()) );

  return pEnv->NewStringUTF( pEnv, mappedName.c_str() );
}

//JavaString HelperVMRuntime::GetJavaStringFromStringObject( JNIEnv * pEnv, jstring libName )
//{
  //JavaString result = JavaString::EmptyString();

  //return HelperTypes::ExtractValueFromStringObject( libName );

  /*jboolean isCopy = false;
  const jbyte *bytes = pEnv->GetStringUTFChars( pEnv, libName, &isCopy );

  try
  {
    result = JavaString::FromByteArray( strlen( reinterpret_cast<const char *>(bytes) ), reinterpret_cast<const uint8_t *>(bytes) );
  }
  catch ( ... )
  {
    pEnv->ReleaseStringUTFChars( pEnv, libName, reinterpret_cast<const char *>(bytes) );
    throw;
  }

  pEnv->ReleaseStringUTFChars( pEnv, libName, reinterpret_cast<const char *>(bytes) );

  return result;*/
//}

void JNICALL HelperVMRuntime::java_lang_VMRuntime_runFinalizationForExit( JNIEnv *pEnv, jobject obj )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMRuntime_runFinalizationForExit()\n" );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>(pEnv);
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>(pInternal->m_pInternal);

  pVirtualMachineState->RunAllFinalizers();
}

void JNICALL HelperVMRuntime::java_lang_VMRuntime_exit( JNIEnv *pEnv, jobject obj, jint statusCode )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMRuntime_exit()\n" );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>(pEnv);
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>(pInternal->m_pInternal);

  pVirtualMachineState->Halt( statusCode );
}

jint JNICALL HelperVMRuntime::java_lang_VMRuntime_nativeLoad( JNIEnv *pEnv, jobject obj, jstring fileName, jobject classLoader )
{
  boost::intrusive_ptr<ObjectReference> pFileName = JNIEnvInternal::ConvertJObjectToObjectPointer( fileName );

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMRuntime_nativeLoad(%s)\n", reinterpret_cast<const char *>(pFileName->ToString().ToUtf8String().c_str()) );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>(pEnv);
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>(pInternal->m_pInternal);

  bool bResult = false;
  jboolean isCopy = false;
  const jbyte *bytes = pEnv->GetStringUTFChars( pEnv, fileName, &isCopy );

  try
  {
    if ( 0 == strcmp( reinterpret_cast<const char *>(bytes), "c:\\javalang.dll" ) )
    {
      pEnv->ReleaseStringUTFChars( pEnv, fileName, reinterpret_cast<const char *>(bytes) );
      return 1;
    }

    if ( 0 == strcmp( reinterpret_cast<const char *>(bytes), "c:\\javanio.dll" ) )
    {
      pEnv->ReleaseStringUTFChars( pEnv, fileName, reinterpret_cast<const char *>(bytes) );
      return 1;
    }

    std::shared_ptr<JavaNativeInterface> pJNI = pVirtualMachineState->GetJavaNativeInterface();
    bResult = pJNI->LoadOsLibrary( reinterpret_cast<const char *>(bytes) );
  }
  catch ( ... )
  {
    pEnv->ReleaseStringUTFChars( pEnv, fileName, reinterpret_cast<const char *>(bytes) );
    throw;
  }

  pEnv->ReleaseStringUTFChars( pEnv, fileName, reinterpret_cast<const char *>(bytes) );

  return bResult ? 1 : 0;
}