#include <chrono>

#include "JavaNativeInterface.h"

#include "ILogger.h"
#include "IVirtualMachineState.h"
#include "IClassLibrary.h"

#include "ObjectReference.h"

#include "Endian.h"
#include "GlobalCatalog.h"
#include "OsFunctions.h"

#include "HelperVMSystem.h"

void SetProperty(bool userCode, const std::string& key, const std::string& value, JNIEnv* pEnv, const jobject& properties, const jmethodID& methodID);

jint JNICALL HelperVMSystem::java_lang_VMSystem_identityHashCode( JNIEnv *pEnv, jobject obj, jobject objToHash )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMSystem_identityHashCode\n" );
#endif // _DEBUG

  return static_cast<jint>( reinterpret_cast<intptr_t>( objToHash ) );
}

void JNICALL HelperVMSystem::java_lang_VMSystem_arraycopy( JNIEnv *pEnv, jobject obj, jobject src, jint srcOffset, jobject dest, jint destOffset, jint length )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMSystem_arraycopy (src=%p, srcOffset=%d, dest=%p, destoffSet=%d, length=%d)\n", src, srcOffset, dest, destOffset, length );
#endif // _DEBUG

  pEnv->JVMX_arraycopy( pEnv, obj, src, srcOffset, dest, destOffset, length );
}

jboolean JNICALL HelperVMSystem::java_lang_VMSystem_isWordsBigEndian( JNIEnv *pEnv, jobject obj )
{
#ifdef _DEBUG
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMSystem_isWordsBigEndian\n" );
#endif // _DEBUG

  return Endian::IsBigEndian();
}

jlong JNICALL HelperVMSystem::java_lang_VMSystem_currentTimeMillis( JNIEnv *pEnv, jobject obj )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: java_lang_VMSystem_currentTimeMillis\n" );
#endif // _DEBUG

  std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
  int64_t millsecondsSinceEpoch = std::chrono::duration_cast<std::chrono::milliseconds>( now.time_since_epoch() ).count();

  return millsecondsSinceEpoch;
}

void JNICALL HelperVMSystem::gnu_classpath_VMSystemProperties_preInit( JNIEnv *pEnv, jobject obj, jobject properties )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get( "Logger" );
  pLogger->LogDebug( "*** Inside native Method: gnu_classpath_VMSystemProperties_preInit\n" );
#endif // _DEBUG

  JNIEnvExported *pInternal = reinterpret_cast<JNIEnvExported *>( pEnv );
  IVirtualMachineState *pVirtualMachineState = reinterpret_cast<IVirtualMachineState *>( pInternal->m_pInternal );

//  pVirtualMachineState->SetUserCodeStarted();

  static const JavaString c_ClassName = JavaString::FromCString( "java/util/Properties" );

  boost::intrusive_ptr<ObjectReference> pJavaLangClass = pVirtualMachineState->FindJavaLangClass( c_ClassName );
  if ( nullptr == pJavaLangClass )
  {
    auto pClass = pVirtualMachineState->InitialiseClass( c_ClassName );

    //if ( !pClass->IsInitialsed() )
    //{
    //  pVirtualMachineState->InitialiseClass( c_ClassName );
    //}

    pJavaLangClass = pVirtualMachineState->CreateJavaLangClassFromClassName( pClass->GetName() );
  }

  jclass jClass = reinterpret_cast<jclass>( JNIEnvInternal::ConvertObjectPointerToJObject( pVirtualMachineState, pJavaLangClass ) );

  jmethodID methodID = pEnv->GetMethodID( pEnv, jClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;" );

  jstring strKey = pEnv->NewStringUTF( pEnv, "file.encoding" );
  jstring strValue = pEnv->NewStringUTF( pEnv, "8859_1" );

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    ObjectReference refKey(strKey);
    pLogger->LogDebug("*** Key=%s\n", reinterpret_cast<const char*>(refKey.GetContainedObject()->ToString().ToUtf8String().c_str()));
    ObjectReference refValue(strValue);
    pLogger->LogDebug("*** Value=%s\n", reinterpret_cast<const char*>(refValue.GetContainedObject()->ToString().ToUtf8String().c_str()));
  }
#endif // _DEBUG

  SetProperty(pVirtualMachineState->HasUserCodeStarted(), "java.vm.name", "JVMX2", pEnv, properties, methodID);
  SetProperty(pVirtualMachineState->HasUserCodeStarted(), "gnu.classpath.vm.shortname", "JVMX2", pEnv, properties, methodID);

  strKey = pEnv->NewStringUTF( pEnv, "java.vendor" );
  strValue = pEnv->NewStringUTF( pEnv, "Walluce Pinkham" );

  pEnv->CallObjectMethod( pEnv, properties, methodID, strKey, strValue );

  strKey = pEnv->NewStringUTF( pEnv, "java.vendor.url" );
  strValue = pEnv->NewStringUTF( pEnv, "https://github.com/WallucePinkham/JVMX2" );

  pEnv->CallObjectMethod( pEnv, properties, methodID, strKey, strValue );

  strKey = pEnv->NewStringUTF( pEnv, "java.vm.version" );
  strValue = pEnv->NewStringUTF( pEnv, "0.0.1" );

  pEnv->CallObjectMethod( pEnv, properties, methodID, strKey, strValue );

  strKey = pEnv->NewStringUTF( pEnv, "java.io.tmpdir" );
  strValue = pEnv->NewStringUTF( pEnv, "c:\\temp\\" );

  pEnv->CallObjectMethod( pEnv, properties, methodID, strKey, strValue );

  strKey = pEnv->NewStringUTF( pEnv, "java.library.path" );
  //strValue = pEnv->NewStringUTF( pEnv, OsFunctions::GetInstance().GetPathAnsi() );
  strValue = pEnv->NewStringUTF( pEnv, "c:\\;c:\\temp\\jvmx;C:\\dev\\JVMX2\\classpath-0.99\\lib" );

  pEnv->CallObjectMethod( pEnv, properties, methodID, strKey, strValue );

  SetProperty(pVirtualMachineState->HasUserCodeStarted(), "path.separator", OsFunctions::GetInstance().GetPathSeparator(), pEnv, properties, methodID);
  SetProperty(pVirtualMachineState->HasUserCodeStarted(), "file.separator", OsFunctions::GetInstance().GetFileSeparator(), pEnv, properties, methodID);
  SetProperty(pVirtualMachineState->HasUserCodeStarted(), "line.separator", OsFunctions::GetInstance().GetLineSeparator(), pEnv, properties, methodID);

  //   strKey = pEnv->NewStringUTF( pEnv, "java.security.manager" );
  //   strValue = pEnv->NewStringUTF( pEnv, "" );
  //
  //   pEnv->CallObjectMethod( pEnv, properties, methodID, strKey, strValue );

  // The mere presense of this setting seems to break URLs, regarding of the value.
  // strKey = pEnv->NewStringUTF( pEnv, "gnu.java.net.nocache_protocol_handlers" );
  // strValue = pEnv->NewStringUTF( pEnv, "false" );

  pEnv->CallObjectMethod( pEnv, properties, methodID, strKey, strValue );

  strKey = pEnv->NewStringUTF( pEnv, "awt.toolkit" );
  strValue = pEnv->NewStringUTF( pEnv, "gnu.java.awt.peer.qt.QtToolkit" );

  pEnv->CallObjectMethod( pEnv, properties, methodID, strKey, strValue );

  strKey = pEnv->NewStringUTF( pEnv, "user.language" );
  strValue = pEnv->NewStringUTF( pEnv, "en" );

  pEnv->CallObjectMethod( pEnv, properties, methodID, strKey, strValue );


  strKey = pEnv->NewStringUTF( pEnv, "user.country" );
  strValue = pEnv->NewStringUTF( pEnv, "US" ); // TODO: This needs to be fixed

  pEnv->CallObjectMethod( pEnv, properties, methodID, strKey, strValue );

  strKey = pEnv->NewStringUTF( pEnv, "user.name" );
  strValue = pEnv->NewStringUTF( pEnv, "Walluce" );

  pEnv->CallObjectMethod( pEnv, properties, methodID, strKey, strValue );

  strKey = pEnv->NewStringUTF( pEnv, "user.home" );
  strValue = pEnv->NewStringUTF( pEnv, "C:\\dev" );

  pEnv->CallObjectMethod( pEnv, properties, methodID, strKey, strValue );

  strKey = pEnv->NewStringUTF( pEnv, "gnu.classpath.home.url" );
  strValue = pEnv->NewStringUTF( pEnv, "file:\\\\C:\\dev" );

  pEnv->CallObjectMethod( pEnv, properties, methodID, strKey, strValue );

  strKey = pEnv->NewStringUTF( pEnv, "os.name" );
  strValue = pEnv->NewStringUTF( pEnv, "Windows 7" ); // TODO: This needs to be done via an API.

  pEnv->CallObjectMethod( pEnv, properties, methodID, strKey, strValue );

  strKey = pEnv->NewStringUTF( pEnv, "java.class.path" );
  strValue = pEnv->NewStringUTF( pEnv, pVirtualMachineState->GetClassPath().c_str() );

  pEnv->CallObjectMethod( pEnv, properties, methodID, strKey, strValue );

  //   strKey = pEnv->NewStringUTF( pEnv, "javax.accessibility.screen_magnifier_present" );
  //   strValue = pEnv->NewStringUTF( pEnv, "false" );
  //
  //   pEnv->CallObjectMethod( pEnv, properties, methodID, strKey, strValue );


  // Awt STUFF
  //Xbootclasspath / a:pja.jar( changing classpath is not enough ).
  //   strKey = pEnv->NewStringUTF( pEnv, "awt.toolkit" );
  //   strValue = pEnv->NewStringUTF( pEnv, "com.eteks.awt.PJAToolkit" );
  //
  //   pEnv->CallObjectMethod( pEnv, properties, methodID, strKey, strValue );
  //
  //   strKey = pEnv->NewStringUTF( pEnv, "java.awt.graphicsenv" );
  //   strValue = pEnv->NewStringUTF( pEnv, "com.eteks.java2d.PJAGraphicsEnvironment" );
  //
  //   pEnv->CallObjectMethod( pEnv, properties, methodID, strKey, strValue );
  //
  //   strKey = pEnv->NewStringUTF( pEnv, "java2d.font.usePlatformFont" );
  //   strValue = pEnv->NewStringUTF( pEnv, "false" );
  //
  //   pEnv->CallObjectMethod( pEnv, properties, methodID, strKey, strValue );
  //
  //   strKey = pEnv->NewStringUTF( pEnv, "java.awt.fonts" );
  //   strValue = pEnv->NewStringUTF( pEnv, "C:\\Program Files (x86)\\Java\\jre1.8.0_31\\lib\\fonts" );
  //
  //   pEnv->CallObjectMethod( pEnv, properties, methodID, strKey, strValue );

  //- D = false : this avoids the class sun.java2d.loops.RasterOutputManager to call the native method getPlatformFontVar(), that may cause a JVM crash.
  //- Djava.awt.fonts = path

  //   pEnv->CallObjectMethod( pEnv, properties, methodID, "file.encoding.pkg", "" );
  //   pEnv->CallObjectMethod( pEnv, properties, methodID, "java.class.path", "TODO" );
  //   pEnv->CallObjectMethod( pEnv, properties, methodID, "java.class.version", "1" );
  //   pEnv->CallObjectMethod( pEnv, properties, methodID, "java.compiler", "" );
  //   pEnv->CallObjectMethod( pEnv, properties, methodID, "java.home", "TODO" );
  //
  //   pEnv->CallObjectMethod( pEnv, properties, methodID, "java.version", "TODO" );
  //   pEnv->CallObjectMethod( pEnv, properties, methodID, "line.separator", "TODO" );
  //   pEnv->CallObjectMethod( pEnv, properties, methodID, "os.name", "TODO" );
  //   pEnv->CallObjectMethod( pEnv, properties, methodID, "os.arch", "TODO" );
  //   pEnv->CallObjectMethod( pEnv, properties, methodID, "os.version", "TODO" );

  //   pEnv->CallObjectMethod( pEnv, properties, methodID, "user.dir", "TODO" );
  //   pEnv->CallObjectMethod( pEnv, properties, methodID, "user.home", "TODO" );
  //   pEnv->CallObjectMethod( pEnv, properties, methodID, "user.language", "TODO" );
  //   pEnv->CallObjectMethod( pEnv, properties, methodID, "user.name", "TODO" );
  //   pEnv->CallObjectMethod( pEnv, properties, methodID, "user.region", "TODO" );
  //   pEnv->CallObjectMethod( pEnv, properties, methodID, "user.timezone", "TODO" );

  auto props = pVirtualMachineState->GetProperties();
  for (auto property : props)
  {
    SetProperty(pVirtualMachineState->HasUserCodeStarted(), property.first, property.second, pEnv, properties, methodID);
  }
}

void SetProperty(bool userCode, const std::string &key, const std::string& value, JNIEnv* pEnv, const jobject& properties, const jmethodID& methodID)
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (userCode)
  {
    std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get("Logger");
    pLogger->LogDebug("*** Key=[%s] Value=[%s]", key.c_str(), value.c_str());
  }
#endif // _DEBUG

  jstring strKey = pEnv->NewStringUTF(pEnv, key.c_str());
  jstring strValue = pEnv->NewStringUTF(pEnv, value.c_str());

  pEnv->CallObjectMethod(pEnv, properties, methodID, strKey, strValue);
}
