
#include "JavaNativeInterface.h"

#include "IVirtualMachineState.h"

#include "GlobalCatalog.h"
#include "OsFunctions.h"
#include "HelperTypes.h"

#include "JavaExceptionConstants.h"

#include "HelperVMInetAddress.h"

jarray JNICALL HelperVMInetAddress::java_net_VMInetAddress_lookupInaddrAny(JNIEnv* pEnv, jclass clazz)
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get("Logger");
  pLogger->LogDebug("*** Inside native Method: java_net_VMInetAddress_lookupInaddrAny()");
#endif // _DEBUG

  JNIEnvExported* pInternal = reinterpret_cast<JNIEnvExported*>(pEnv);
  IVirtualMachineState* pVirtualMachineState = reinterpret_cast<IVirtualMachineState*>(pInternal->m_pInternal);

  uint8_t bytes[4] = {0};
  OsFunctions::GetInstance().GetInetAddrAnyIpV4(bytes);

  boost::intrusive_ptr<ObjectReference> pResult = HelperTypes::CreateArray(e_JavaArrayTypes::Byte, 4);
  auto pInternalArray = pResult->GetContainedArray();
  
  pInternalArray->SetAt(0, JavaByte::FromHostInt8(bytes[0]));
  pInternalArray->SetAt(1, JavaByte::FromHostInt8(bytes[1]));
  pInternalArray->SetAt(2, JavaByte::FromHostInt8(bytes[2]));
  pInternalArray->SetAt(3, JavaByte::FromHostInt8(bytes[3]));

  return JNIEnvInternal::ConvertArrayPointerToJArray(pVirtualMachineState, pResult);
}

jstring JNICALL HelperVMInetAddress::java_net_VMInetAddress_getLocalHostname(JNIEnv* pEnv, jclass clazz)
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get("Logger");
  pLogger->LogDebug("*** Inside native Method: java_net_VMInetAddress_getLocalHostname()");
#endif // _DEBUG

  JNIEnvExported* pInternal = reinterpret_cast<JNIEnvExported*>(pEnv);
  IVirtualMachineState* pVirtualMachineState = reinterpret_cast<IVirtualMachineState*>(pInternal->m_pInternal);

  std::string hostName = OsFunctions::GetInstance().GetHostName();

  auto pResult = pVirtualMachineState->CreateStringObject(hostName.c_str());

  return JNIEnvInternal::ConvertObjectPointerToJString(pVirtualMachineState, pResult);
}

jarray JNICALL HelperVMInetAddress::java_net_VMInetAddress_aton(JNIEnv* pEnv, jclass clazz, jstring stringVal)
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get("Logger");
  pLogger->LogDebug("*** Inside native Method: java_net_VMInetAddress_aton()");
#endif // _DEBUG

  JNIEnvExported* pInternal = reinterpret_cast<JNIEnvExported*>(pEnv);
  IVirtualMachineState* pVirtualMachineState = reinterpret_cast<IVirtualMachineState*>(pInternal->m_pInternal);

  boost::intrusive_ptr<ObjectReference> pTypeNameStringObject = JNIEnvInternal::ConvertJObjectToObjectPointer(stringVal);
  JavaString finalStringValue = HelperTypes::ExtractValueFromStringObject(pTypeNameStringObject);

  std::string utf8Str = finalStringValue.ToUtf8String();
  const char* pIpString = utf8Str.c_str();
  uint8_t bytes[4] = { 0 };

  if (!OsFunctions::GetInstance().Ip4StringToBytes(pIpString, bytes))
  {
    return nullptr;
  }

  auto pResult = pVirtualMachineState->CreateArray(e_JavaArrayTypes::Byte, 4);

  auto pInternalArray = pResult->GetContainedArray();
  pInternalArray->SetAt(0, JavaByte::FromHostInt8(bytes[0]));
  pInternalArray->SetAt(1, JavaByte::FromHostInt8(bytes[1]));
  pInternalArray->SetAt(2, JavaByte::FromHostInt8(bytes[2]));
  pInternalArray->SetAt(3, JavaByte::FromHostInt8(bytes[3]));

  return JNIEnvInternal::ConvertArrayPointerToJArray(pVirtualMachineState, pResult);
}