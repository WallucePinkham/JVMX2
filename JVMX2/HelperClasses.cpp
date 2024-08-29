
#include "JVMXException.h"

#include "ILogger.h"
#include "IClassLibrary.h"
#include "GlobalCatalog.h"
#include "JavaExceptionConstants.h"
#include "InvalidArgumentException.h"
#include "InvalidStateException.h"

#include "HelperClasses.h"



bool HelperClasses::DoesClassImplementInterface(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, 
                                                std::shared_ptr<JavaClass> pClass, 
                                                boost::intrusive_ptr<JavaString> nameOfInterface)
{
  return DoesClassImplementInterface(pVirtualMachineState.get(), pClass, nameOfInterface.get());
}

bool HelperClasses::DoesClassImplementInterface(IVirtualMachineState* pVirtualMachineState,
                                                std::shared_ptr<JavaClass> pClass,
                                                const JavaString* const nameOfInterface)
{
  for (size_t i = 0; i < pClass->GetInterfacesCount(); ++i)
  {
    if (pClass->GetInterfaceName(i) == *nameOfInterface)
    {
      return true;
    }
  }

  if (nullptr != pClass->GetSuperClass())
  {
    return DoesClassImplementInterface(pVirtualMachineState, ResolveClass(pVirtualMachineState, *pClass->GetSuperClass()->GetName()), nameOfInterface);
  }

  return false;
}

std::shared_ptr<JavaClass> HelperClasses::ResolveClass(IVirtualMachineState* pVirtualMachineState,
                                                       const JavaString& className)
{
  std::shared_ptr<ILogger> pLogger = GlobalCatalog::GetInstance().Get("Logger");
  std::shared_ptr<IClassLibrary> pClassLib = GlobalCatalog::GetInstance().Get("ClassLibrary");

  std::shared_ptr<JavaClass> pClassFile = pClassLib->FindClass(className);
  if (nullptr == pClassFile)
  {
    try
    {
      pClassFile = pVirtualMachineState->InitialiseClass(className);
    }
    catch (JVMXException& ex)
    {
      pLogger->LogWarning(__FUNCTION__ " - Throwing linkage exception because class could not be loaded: %s", ex.what());
      ThrowJavaException(pVirtualMachineState, c_JavaLinkageErrorException);
    }
  }

  if (nullptr != pClassFile)
  {
    if ((!pClassFile->IsPublic()) && (pClassFile->GetPackageName() != pVirtualMachineState->GetCurrentClass()->GetPackageName()))
    {
      ThrowJavaException(pVirtualMachineState, c_JavaIllegalAccessErrorException);
      pClassFile = nullptr;
    }
  }

  return pClassFile;
}

std::shared_ptr<JavaClass> HelperClasses::ResolveClass(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, const JavaString& className)
{
  return ResolveClass(pVirtualMachineState.get(), className);
}

void HelperClasses::ThrowJavaException(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, const JVMX_ANSI_CHAR_TYPE* javaExceptionName)
{
  ThrowJavaException(pVirtualMachineState.get(), javaExceptionName);
}

void HelperClasses::ThrowJavaException(IVirtualMachineState* pVirtualMachineState, const JVMX_ANSI_CHAR_TYPE* javaExceptionName)
{
  if (nullptr == javaExceptionName)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Java Exception name was NULL");
  }

  auto pClass = pVirtualMachineState->InitialiseClass(JavaString::FromCString(javaExceptionName));
  if (nullptr == pClass)
  {
    throw InvalidStateException(__FUNCTION__ " - Java exception class could not be loaded.");
  }

  //if (nullptr != pMessage)
  //{
  //  boost::intrusive_ptr<ObjectReference> pMessage = pVirtualMachineState->CreateStringObject(JavaString::FromCString(message));
  //  boost::intrusive_ptr<ObjectReference> pExceptionObject = pVirtualMachineState->CreateObject(pClass);

  //  pVirtualMachineState->PushOperand(pExceptionObject);
  //  pVirtualMachineState->PushOperand(pMessage);

  //  std::shared_ptr<MethodInfo> pMethodInfo = pClass->GetMethodByNameAndType(c_InstanceInitialisationMethodName, JavaString::FromCString(u"(Ljava/lang/String;)V"));
  //  pVirtualMachineState->ExecuteMethod(*pClassName, c_InstanceInitialisationMethodName, JavaString::FromCString(u"(Ljava/lang/String;)V"), pMethodInfo);
  //}

  auto pExceptionObject = pVirtualMachineState->CreateAndInitialiseObject(pClass);
  ThrowJavaExceptionInternal(pVirtualMachineState, pExceptionObject);
}

void HelperClasses::ThrowJavaExceptionInternal(IVirtualMachineState* pVirtualMachineState, boost::intrusive_ptr<ObjectReference> pException)
{
  pVirtualMachineState->SetExceptionThrown(pException);
}

bool HelperClasses::IsSuperClassOf(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState,
                                   boost::intrusive_ptr<JavaString> pPossibleSuperClassName,
                                   boost::intrusive_ptr<JavaString> pDerivedClassName)
{
  return IsSuperClassOf(pVirtualMachineState.get(), pPossibleSuperClassName.get(), pDerivedClassName.get());
}

bool HelperClasses::IsSuperClassOf(IVirtualMachineState* pVirtualMachineState,
                                   const JavaString* const pPossibleSuperClassName,
                                   const JavaString* const pDerivedClassName)
{
  auto pDerivedClass = pVirtualMachineState->InitialiseClass(*pDerivedClassName);
 /* if (!pDerivedClass->IsInitialsed())
  {
    pVirtualMachineState->InitialiseClass(pDerivedClass);
  }*/

  if (nullptr == pDerivedClass->GetSuperClass())
  {
    return false;
  }

  //auto pSuperClass = GetClassLibrary()->FindClass( *(pDerivedClass->GetSuperClassName()) );
  std::shared_ptr<JavaClass> pSuperClass = pDerivedClass->GetSuperClass();
  while (nullptr != pSuperClass)
  {
    if (*pSuperClass->GetName() == *pPossibleSuperClassName)
    {
      return true;
    }

    if (nullptr == pSuperClass->GetSuperClass())
    {
      return false;
    }

    pSuperClass = pSuperClass->GetSuperClass();
  }

  return false; 
}