
#include "JavaNativeInterface.h"

#include "IVirtualMachineState.h"

#include "GlobalCatalog.h"
#include "OsFunctions.h"

#include "HelperTypes.h"
#include "HelperVMFile.h"

jboolean JNICALL HelperVMFile::java_io_VMFile_isDirectory(JNIEnv* pEnv, jobject obj, jstring path)
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get("Logger");
  pLogger->LogDebug("*** Inside native Method: java_java_io_VMFile_isDirectory( 0x%p )", path);
#endif // _DEBUG

  JNIEnvExported* pInternal = reinterpret_cast<JNIEnvExported*>(pEnv);
  IVirtualMachineState* pVirtualMachineState = reinterpret_cast<IVirtualMachineState*>(pInternal->m_pInternal);

  boost::intrusive_ptr<ObjectReference> pStringObject = JNIEnvInternal::ConvertJObjectToObjectPointer(path);

  JavaString stringValue = HelperTypes::ExtractValueFromStringObject(pStringObject);
  if (OsFunctions::GetInstance().IsDirectory(stringValue.ToUtf8String().c_str()))
  {
    return JNI_TRUE;
  }

  return JNI_FALSE;
}

jboolean JNICALL HelperVMFile::java_io_VMFile_exists(JNIEnv* pEnv, jobject obj, jstring path)
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get("Logger");
  pLogger->LogDebug("*** Inside native Method: java_io_VMFile_exists( 0x%p )", path);
#endif // _DEBUG

  JNIEnvExported* pInternal = reinterpret_cast<JNIEnvExported*>(pEnv);
  IVirtualMachineState* pVirtualMachineState = reinterpret_cast<IVirtualMachineState*>(pInternal->m_pInternal);

  jboolean result = JNI_FALSE;
  jboolean isCopy = false;
  const jbyte* bytes = pEnv->GetStringUTFChars(pEnv, path, &isCopy);

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pLogger->LogDebug("Checking existence of path: %s", reinterpret_cast<const char*>(bytes));
  }
#endif // _DEBUG

  try
  {
    struct _stat buffer = { 0 };
    if (0 == _stat(reinterpret_cast<const char*>(bytes), &buffer))
    {
      result = JNI_TRUE;
    }
  }
  catch (...)
  {
    pEnv->ReleaseStringUTFChars(pEnv, path, reinterpret_cast<const char*>(bytes));
    throw;
  }

  pEnv->ReleaseStringUTFChars(pEnv, path, reinterpret_cast<const char*>(bytes));

  return result;
}

jobject JNICALL HelperVMFile::java_io_VMFile_toCanonicalForm(JNIEnv* pEnv, jobject obj, jstring path)
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get("Logger");
  pLogger->LogDebug("*** Inside native Method: java_io_VMFile_toCanonicalForm( 0x%p )", path);
#endif // _DEBUG

  JNIEnvExported* pInternal = reinterpret_cast<JNIEnvExported*>(pEnv);
  IVirtualMachineState* pVirtualMachineState = reinterpret_cast<IVirtualMachineState*>(pInternal->m_pInternal);

  boost::intrusive_ptr<ObjectReference> pStringObject = JNIEnvInternal::ConvertJObjectToObjectPointer(path);

  JavaString stringValue = HelperTypes::ExtractValueFromStringObject(pStringObject);

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pLogger->LogDebug("Getting canonical form for %s", stringValue.ToUtf8String().c_str());
  }
#endif // _DEBUG

  std::u16string canonicalForm = OsFunctions::GetInstance().GetCanonicalFormUtf16(stringValue.ToUtf16String().c_str());
  JavaString result = JavaString::FromCString(canonicalForm.c_str());

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pLogger->LogDebug("Canonical form is %s", result.ToUtf8String().c_str());
  }
#endif // _DEBUG

  boost::intrusive_ptr<ObjectReference> pName = pVirtualMachineState->CreateStringObject(result);

  return JNIEnvInternal::ConvertObjectPointerToJObject(pVirtualMachineState, pName);
}