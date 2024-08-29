
#include "GlobalConstants.h"
#include "GlobalCatalog.h"

#include "NotImplementedException.h"
#include "InvalidStateException.h"
#include "InternalErrorException.h"
#include "DefaultClassLoader.h"
#include "IClassLibrary.h"
#include "VirtualMachine.h"

#include "HelperTypes.h"
#include "HelperClasses.h"
#include "JavaExceptionConstants.h"

#include "JavaNativeInterface.h"

#include "HelperVMClassLoader.h"

jobject JNICALL HelperVMClassLoader::java_lang_VMClassLoader_getPrimitiveClass(JNIEnv* pEnv, jobject obj, jchar typeAsChar)
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get("Logger");
  pLogger->LogDebug("*** Inside native Method: java_lang_VMClassLoader_getPrimitiveClass(%u)", typeAsChar);
#endif // _DEBUG

  JNIEnvExported* pInternal = reinterpret_cast<JNIEnvExported*>(pEnv);
  IVirtualMachineState* pVirtualMachineState = reinterpret_cast<IVirtualMachineState*>(pInternal->m_pInternal);

  boost::intrusive_ptr<ObjectReference> pResult = VirtualMachine::GetPrimitiveClass(typeAsChar, pVirtualMachineState);

  //JavaClass *pClass = reinterpret_cast<JavaClass *>(clazz);
  return JNIEnvInternal::ConvertObjectPointerToJObject(pVirtualMachineState, pResult);
}

jobject JNICALL HelperVMClassLoader::java_lang_VMClassLoader_defineClass(
  JNIEnv* pEnv,
  jobject obj,
  jobject classLoaderObj,
  jobject name,
  jarray data,
  jint offset,
  jint len,
  jobject pd)
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get("Logger");
  pLogger->LogDebug("*** Inside native Method: java_lang_VMClassLoader_defineClass()");
#endif // _DEBUG

  JNIEnvExported* pInternal = reinterpret_cast<JNIEnvExported*>(pEnv);
  IVirtualMachineState* pVirtualMachineState = reinterpret_cast<IVirtualMachineState*>(pInternal->m_pInternal);

  if (offset != 0)
  {
    throw NotImplementedException(__FUNCTION__ " - Not implented. Non-zero offsets are not yet supported.");
  }

  boost::intrusive_ptr<ObjectReference> pClassLoader = JNIEnvInternal::ConvertJObjectToObjectPointer(classLoaderObj);

  DefaultClassLoader loader;

  auto pObject = JNIEnvInternal::ConvertJArrayToArrayPointer(data);
  auto pArray = pObject->GetContainedArray();
  auto buffer = pArray->ConvertByteArrayToBuffer();

  if (buffer.GetByteLength() != len - offset)
  {
    throw InternalErrorException(__FUNCTION__ " - Something went wrong reading array of bytes into a buffer.");
  }

  BigEndianStream stream = BigEndianStream::FromDataBuffer(buffer);

  auto pClass = loader.LoadClass(stream, pClassLoader);

  if (nullptr == pClass)
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaClassFormatError);
    return JNIEnvInternal::ConvertNullPointerToJObject();
  }

  if (!pClass->IsInitialsed())
  {
    pVirtualMachineState->InitialiseClass(*pClass->GetName().get());
  }

  std::shared_ptr<IClassLibrary> lib = GlobalCatalog::GetInstance().Get("ClassLibrary");
  lib->AddClass(pClass);

  auto pResult = pVirtualMachineState->CreateJavaLangClassFromClassName(pClass->GetName());

#if defined(_DEBUG)
  boost::intrusive_ptr<ObjectReference> pTypeNameStringObject = JNIEnvInternal::ConvertJObjectToObjectPointer(name);
  JavaString finalStringValue = HelperTypes::ExtractValueFromStringObject(pTypeNameStringObject);

  if (*pClass->GetName().get() != finalStringValue)
  {
    pLogger->LogWarning("Loaded Class Name [%s] does not match expected class name [%s].",
      pClass->GetName().get()->ToCharacterArray(),
      finalStringValue.ToCharacterArray());
  }
#endif // _DEBUG

  return JNIEnvInternal::ConvertObjectPointerToJObject(pVirtualMachineState, pResult);
}