
#include "JavaNativeInterface.h"
#include "GlobalCatalog.h"

#include "ILogger.h"
#include "StringPool.h"
#include "IVirtualMachineState.h"

#include "HelperTypes.h"

#include "HelperVMString.h"

boost::intrusive_ptr<ObjectReference> GetFromStringPool(const JavaString &string)
{
  std::shared_ptr<StringPool> pStringPool = GlobalCatalog::GetInstance().Get("StringPool");
  auto pObject = pStringPool->Get(string);
  return pObject;
}

void AddToStringPool(const JavaString &string, boost::intrusive_ptr<ObjectReference> pObject)
{
  std::shared_ptr<StringPool> pStringPool = GlobalCatalog::GetInstance().Get("StringPool");
  pStringPool->Add(string, pObject);
}

jstring JNICALL HelperVMString::java_lang_VMString_intern(JNIEnv* pEnv, jobject obj, jobject string)
{
  boost::intrusive_ptr<ObjectReference> pStringObject = JNIEnvInternal::ConvertJObjectToObjectPointer(string);
  JavaString javaString = HelperTypes::ExtractValueFromStringObject(pStringObject);

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get("Logger");
  pLogger->LogDebug("*** Inside native Method: Java_java_lang_VMString_intern(%s)\n", javaString.ToUtf8String().c_str());
#endif // _DEBUG

  JNIEnvExported* pInternal = reinterpret_cast<JNIEnvExported*>(pEnv);
  IVirtualMachineState* pVirtualMachineState = reinterpret_cast<IVirtualMachineState*>(pInternal->m_pInternal);

  boost::intrusive_ptr<ObjectReference> pObject = GetFromStringPool(javaString);
  if (nullptr == pObject)
  {
    pObject = pVirtualMachineState->CreateStringObject(javaString);
    AddToStringPool(javaString, pObject);
  }

  return JNIEnvInternal::ConvertObjectPointerToJString(pVirtualMachineState, pObject);
}