#include <cstdlib>
#include <cfloat>
#include <cstring>
#include <atomic>
#include <memory>
#include <vector>
#include <map>
#include <thread>

#include "IClassLibrary.h"
#include "IVirtualMachineState.h"
#include "ILogger.h"
#include "JavaNativeInterface.h"
#include "UnsupportedTypeException.h"
#include "InvalidStateException.h"
#include "TypeMismatchException.h"
#include "InvalidArgumentException.h"
#include "NotImplementedException.h"
#include "AssertionFailedException.h"
#include "FileDoesNotExistException.h"
#include "JavaExceptionConstants.h"
#include "ClassAttributeCode.h"
#include "JavaTypes.h"
#include "JavaClass.h"
#include "TypeParser.h"
#include "ConstantPoolEntry.h"
#include "MethodInfo.h"
#include "JavaOpCodes.h"
#include "GlobalCatalog.h"
#include "StringPool.h"
#include "ObjectReference.h"
#include "HelperClasses.h"

#include "BasicExecutionEngine.h"

extern const JavaString c_ClassInitialisationMethodName;
extern const JavaString c_InstanceInitialisationMethodName;

extern const JavaString c_MethodTypeClassName;
extern const JavaString c_ThrowableClassName;

extern const JavaString c_UncaughtExceptionMethodName;
extern const JavaString c_UncaughtExceptionMethodType;


e_IncreaseCallStackDepth BasicExecutionEngine::ExecuteOpCodeInvokeStatic(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  // The unsigned indexbyte1 and indexbyte2 are used to construct an index into the runtime constant pool of the current class

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
    pVirtualMachineState->LogCallStack();
  }
#endif // _DEBUG

  boost::intrusive_ptr<JavaString> pClassName = new JavaString(JavaString::EmptyString());
  std::shared_ptr<MethodInfo> pMethodInfo = ResolveMethodReference(pVirtualMachineState, ReadIndex(pVirtualMachineState), *pClassName);
  if (!pMethodInfo->IsStatic() || pMethodInfo->IsAbstract())
  {
    pVirtualMachineState->LogLocalVariables();

#ifdef _DEBUG
    GetLogger()->LogDebug(__FUNCTION__ " - Throwing exception because method is not static or is abstract: %s", pMethodInfo->GetName()->ToUtf8String().c_str());
#endif // _DEBUG

    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaIncompatibleClassChangeErrorException);
    return e_IncreaseCallStackDepth::No;
  }

  // On successful resolution of the field, the class or interface that declared the resolved field is initialized if that class or
  // interface has not already been initialized.
  if (!pVirtualMachineState->IsClassInitialised(*pClassName))
  {
    pVirtualMachineState->InitialiseClass(*pClassName);
  }

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Method type: %s.", pMethodInfo->GetType()->ToUtf8String().c_str());
  }
#endif // _DEBUG

  // TODO: Pop nargs from operand stack where the   number, type, and order of the values must be consistent with the descriptor of the resolved method

  if (pMethodInfo->IsSynchronised())
  {
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
      GetLogger()->LogDebug("Static Method %s is synchronized.", pMethodInfo->GetName()->ToUtf8String().c_str());
    }
#endif // _DEBUG

    pVirtualMachineState->PushMonitor(pMethodInfo->GetClass()->MonitorEnter(pMethodInfo->GetFullName().ToUtf8String().c_str()));
  }

  if (pMethodInfo->IsNative())
  {
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
      GetLogger()->LogDebug("Method implementation is native: %s type: %s.", pMethodInfo->GetName()->ToUtf8String().c_str(), pMethodInfo->GetType()->ToUtf8String().c_str());
    }
#endif // _DEBUG

    TypeParser::ParsedMethodType type = TypeParser::ParseMethodType(*(pMethodInfo->GetType()));
    boost::intrusive_ptr<ObjectReference> pJavaLangClass = pVirtualMachineState->CreateJavaLangClassFromClassName(pClassName);

    std::shared_ptr<JavaNativeInterface> pJNI = pVirtualMachineState->GetJavaNativeInterface();

    //pVirtualMachineState->PushState( *( pMethodInfo->GetClass()->GetName() ), *( pMethodInfo->GetName() ), *( pMethodInfo->GetType() ), pMethodInfo );
    pJNI->ExecuteFunctionStatic(JavaNativeInterface::MakeJNINameWithoutArgumentDescriptor(pClassName, pMethodInfo->GetName()), pMethodInfo->GetType(), pJavaLangClass);
    //pVirtualMachineState->PopState();

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
      pVirtualMachineState->LogOperandStack();
    }
#endif

    if (pMethodInfo->IsSynchronised())
    {
      pVirtualMachineState->PopMonitor();
      pMethodInfo->GetClass()->MonitorExit(pMethodInfo->GetFullName().ToUtf8String().c_str());
    }

    return e_IncreaseCallStackDepth::No;
  }
  else
  {
    // values are consecutively made the values of local variables of the new frame, with arg1 in local variable 0 (or, if arg1 is of type long or double, in local variables 0 and 1) and so on
    //pVirtualMachineState->PushState( *pClassName, *(pMethodInfo->GetName()), *(pMethodInfo->GetType()) );
    pVirtualMachineState->PushState(*(pMethodInfo->GetClass()->GetName()), *(pMethodInfo->GetName()), *(pMethodInfo->GetType()), pMethodInfo);
    pVirtualMachineState->SetCodeSegment(pMethodInfo->GetCodeInfo());
    pVirtualMachineState->SetupLocalVariables(pMethodInfo);
  }

  return e_IncreaseCallStackDepth::Yes;
}

std::shared_ptr<MethodInfo> BasicExecutionEngine::ResolveMethodReference(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, uint16_t methodIndex, JavaString& className)
{
  // The runtime constant pool item at that index must be a symbolic reference to a field
  std::shared_ptr<ConstantPoolEntry> methodEntry = pVirtualMachineState->GetConstantFromCurrentClass(methodIndex);
  if (e_ConstantPoolEntryTypeMethodReference != methodEntry->GetType())
  {
    GetLogger()->LogError(__FUNCTION__ " - Could not resolve method reference: %hu", methodIndex);
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaIncompatibleClassChangeErrorException);
    return nullptr;
  }

  const ConstantPoolMethodReference* pMethodRef = dynamic_cast<const ConstantPoolMethodReference*>(methodEntry->GetValue());
  if (nullptr == pMethodRef)
  {
    throw TypeMismatchException(__FUNCTION__ " - Could not convert constant pool entry to field reference.");
  }

  // The referenced field is resolved
  boost::intrusive_ptr<JavaString> pClassName = pMethodRef->GetClassName();
  className = *pClassName;

  std::shared_ptr<MethodInfo> pMethodInfo = pVirtualMachineState->ResolveMethodOnClass(pClassName, pMethodRef);

  return pMethodInfo;
}

void BasicExecutionEngine::ExecuteOpCodeMonitorEnter(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  auto pOperand = pVirtualMachineState->PopOperand();

  if (e_JavaVariableTypes::Object == pOperand->GetVariableType())
  {
    boost::intrusive_ptr<ObjectReference> pReference = boost::dynamic_pointer_cast<ObjectReference>(pOperand);
    if (nullptr == pReference)
    {
      throw InvalidStateException(__FUNCTION__ " - Expected object on operand stack.");
    }

#ifdef _DEBUG
    if (pReference->GetContainedObject()->GetClass()->GetName()->Contains(u"VMThread"))
    {
      //__asm int 3;
    }
#endif // _DEBUG

    pReference->GetContainedObject()->MonitorEnter(pVirtualMachineState->GetCurrentMethodInfo()->GetFullName().ToUtf8String().c_str());
  }
  else if (e_JavaVariableTypes::Array == pOperand->GetVariableType())
  {
    boost::intrusive_ptr<ObjectReference> pReference = boost::dynamic_pointer_cast<ObjectReference>(pOperand);
    if (nullptr == pReference)
    {
      throw InvalidStateException(__FUNCTION__ " - Expected array on operand stack.");
    }

    pReference->GetContainedArray()->MonitorEnter(pVirtualMachineState->GetCurrentClassAndMethodName().ToUtf8String().c_str());
  }
  else if (e_JavaVariableTypes::ClassReference == pOperand->GetVariableType())
  {
    boost::intrusive_ptr<JavaClassReference> pReference = boost::dynamic_pointer_cast<JavaClassReference>(pOperand);
    if (nullptr == pReference)
    {
      throw InvalidStateException(__FUNCTION__ " - Expected class on operand stack.");
    }

    pReference->GetClassFile()->MonitorEnter(pVirtualMachineState->GetCurrentClassAndMethodName().ToUtf8String().c_str());
  }
  else
  {
    throw InvalidArgumentException(__FUNCTION__ " - Expected object, class or array on operand stack.");
  }
}

e_IncreaseCallStackDepth BasicExecutionEngine::ExecuteOpCodeInvokeVirtual(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  std::shared_ptr<ConstantPoolEntry> pConstant = pVirtualMachineState->GetConstantFromCurrentClass(ReadIndex(pVirtualMachineState));
  if (pConstant->GetType() != e_ConstantPoolEntryTypeMethodReference)
  {
    pVirtualMachineState->LogCallStack();
    throw InvalidStateException(__FUNCTION__ " - Expected Method reference.");
  }

  return ExecuteVirtualMethod(pVirtualMachineState, pConstant);
}

e_IncreaseCallStackDepth BasicExecutionEngine::ExecuteOpCodeInvokeSpecial(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  ConstantPoolIndex index = ReadIndex(pVirtualMachineState);

  std::shared_ptr<ConstantPoolMethodReference> pMethodRef = pVirtualMachineState->GetConstantFromCurrentClass(index)->AsMethodReference();
  std::shared_ptr<MethodInfo> pMethodInfo = pVirtualMachineState->ResolveMethodOnClass(pMethodRef->GetClassName(), pMethodRef.get());

  if (nullptr == pMethodInfo)
  {
    throw InvalidStateException(__FUNCTION__ " - Could not resolve method.");
  }

  // Finally, if the resolved method is protected, and it is a member of a superclass of the current class, and the method is not
  // declared in the same runtime package as the current class, then the class of *objectref* must be either the current class or a subclass of the current class.
  if (pMethodInfo->IsProtected())
  {
    //     std::shared_ptr<JavaClassFile> pSuperClass = ResolveClass( *(pVirtualMachineState->GetCurrentClass()->GetSuperClassName()) );
    //     if ( pSuperClass->GetMethodByName( *(pMethodRef->GetName()) ) )
    //     {
    //       if ( pVirtualMachineState->GetCurrentClass()->GetPackageName() != pMethodInfo-> )
    //     }

    // TODO: Fix this.
  }

  // Next, the resolved method is selected for invocation unless all of the following conditions are true:
  // - The ACC_SUPER flag( Table 4.1 ) is set for the current class.
  // - The class of the resolved method is a superclass of the current class.
  // - The resolved method is not an instance initialization method
  std::shared_ptr<MethodInfo> pFinalMethod = pMethodInfo;
  std::shared_ptr<JavaClass> pFinalClass(GetClassLibrary()->FindClass(*pMethodRef->GetClassName()));

  if (pVirtualMachineState->GetCurrentClass()->IsSuper() && IsSuperClassOfCurrentClass(pVirtualMachineState, pMethodRef->GetClassName()) && *(pMethodRef->GetName()) != c_InstanceInitialisationMethodName)
  {
    // If the above conditions are true, the actual method to be invoked let C be the direct superclass of the current class
    std::shared_ptr<JavaClass> pSuperClass = pVirtualMachineState->GetCurrentClass()->GetSuperClass();
    if (nullptr != pSuperClass)
    {
      // If C contains a declaration for an instance method with the same name and descriptor as the resolved method, then this method will be invoked.
      pFinalMethod = pSuperClass->GetMethodByNameAndType(*(pMethodRef->GetName()), *pMethodRef->GetType());
      pFinalClass = pSuperClass;
      if (nullptr == pFinalMethod)
      {
        // Otherwise, if C has a superclass, this same lookup procedure is performed recursively using the direct superclass of C.The
        // method to be invoked is the result of the recursive invocation   of this lookup procedure.
        pSuperClass = pSuperClass->GetSuperClass();
        while (nullptr != pSuperClass)
        {
          pFinalMethod = pSuperClass->GetMethodByNameAndType(*(pMethodRef->GetName()), *pMethodRef->GetType());

          if (nullptr != pFinalMethod)
          {
            pFinalClass = pSuperClass;
            break;
          }

          pSuperClass = pSuperClass->GetSuperClass();
        }

        if (nullptr == pFinalMethod)
        {
          HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaAbstractMethodErrorException);
          return e_IncreaseCallStackDepth::No;
        }
      }
    }
  }

  boost::intrusive_ptr<JavaString> pFinalMethodName = pFinalMethod->GetName();
  boost::intrusive_ptr<JavaString> pFinalMethodType = pFinalMethod->GetType();
  boost::intrusive_ptr<JavaString> pFinalClassName = pFinalClass->GetName();

  if (!pFinalMethod->IsNative())
  {
    //If the method is not native, the nargs argument values and objectref are popped from the operand stack.

    pVirtualMachineState->PushState(*pFinalClassName, *pFinalMethodName, *pFinalMethodType, pFinalMethod);
    boost::intrusive_ptr<ObjectReference> pObject = pVirtualMachineState->SetupLocalVariables(pFinalMethod);

    if (pFinalMethod->IsSynchronised())
    {
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
      if (pVirtualMachineState->HasUserCodeStarted())
      {
        GetLogger()->LogDebug("Special Method is synchronized _ 3: %s.", pFinalMethodType->ToUtf8String().c_str());
      }
#endif // _DEBUG
      pVirtualMachineState->PushMonitor(pObject->GetContainedObject()->MonitorEnter(pFinalMethod->GetFullName().ToUtf8String().c_str()));
    }

    pVirtualMachineState->UpdateCurrentClassName(pFinalClassName);
    pVirtualMachineState->SetCodeSegment(pFinalMethod->GetCodeInfo());

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
      GetLogger()->LogDebug("Executing Method (special): %s::%s", pFinalClassName->ToUtf8String().c_str(), pFinalMethodName->ToUtf8String().c_str());
    }
#endif // _DEBUG

    return e_IncreaseCallStackDepth::Yes;
  }

  // Mehtod is native

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Method implementation is native: %s type: %s.", pFinalMethodName->ToUtf8String().c_str(), pFinalMethodType->ToUtf8String().c_str());
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  //The nargs argument values and objectref are popped from the operand stack and are passed as parameters to the code that implements the method.
  TypeParser::ParsedMethodType parsedType = TypeParser::ParseMethodType(*pFinalMethodType);
  uint8_t objectIndex = static_cast<uint8_t>(parsedType.parameters.size() + 1);

  boost::intrusive_ptr<ObjectReference> pObject = boost::dynamic_pointer_cast<ObjectReference>(pVirtualMachineState->PeekOperandFromBack(objectIndex));

  if (nullptr == pObject)
  {
    throw InvalidStateException(__FUNCTION__ " - Could not find object on the operand stack.");
  }

  if (pFinalMethod->IsStatic())
  {
    if (pFinalMethod->IsSynchronised())
    {
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
      if (pVirtualMachineState->HasUserCodeStarted())
      {
        GetLogger()->LogDebug("Special Method is synchronized _ 1: %s.", pFinalMethodType->ToUtf8String().c_str());
      }
#endif // _DEBUG
      pVirtualMachineState->PushMonitor(pFinalClass->MonitorEnter(pFinalMethod->GetFullName().ToUtf8String().c_str()));
    }

    std::shared_ptr<JavaNativeInterface> pJNI = pVirtualMachineState->GetJavaNativeInterface();

    //pVirtualMachineState->PushState( *( pFinalMethod->GetClass()->GetName() ), *( pFinalMethod->GetName() ), *( pFinalMethod->GetType() ), pFinalMethod );
    //pVirtualMachineState->SetupLocalVariables( pFinalMethod );

    pJNI->ExecuteFunctionStatic(JavaNativeInterface::MakeJNINameWithoutArgumentDescriptor(pFinalClassName, pFinalMethodName), pFinalMethodType, pObject);

    //pVirtualMachineState->PopState();

    if (pFinalMethod->IsSynchronised())
    {
      pVirtualMachineState->PopMonitor()->Unlock(pFinalMethod->GetFullName().ToUtf8String().c_str());
    }
  }
  else
  {
    if (pFinalMethod->IsSynchronised())
    {
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
      if (pVirtualMachineState->HasUserCodeStarted())
      {
        GetLogger()->LogDebug("Method is synchronized _ 2 : %s.", pFinalMethodType->ToUtf8String().c_str());
      }
#endif // _DEBUG
      pVirtualMachineState->PushMonitor(pObject->GetContainedObject()->MonitorEnter(pFinalMethod->GetFullName().ToUtf8String().c_str()));
    }

    std::shared_ptr<JavaNativeInterface> pJNI = pVirtualMachineState->GetJavaNativeInterface();

    //pVirtualMachineState->PushState( *( pMethodInfo->GetClass()->GetName() ), *( pMethodInfo->GetName() ), *( pMethodInfo->GetType() ), pMethodInfo );
    //pVirtualMachineState->SetupLocalVariables( pFinalMethod );

    pJNI->ExecuteFunction(JavaNativeInterface::MakeJNINameWithoutArgumentDescriptor(pFinalClassName, pFinalMethodName), pFinalMethodType, pObject);

    //pVirtualMachineState->PopState();

    if (pFinalMethod->IsSynchronised())
    {
      pVirtualMachineState->PopMonitor()->Unlock(pFinalMethod->GetFullName().ToUtf8String().c_str());
    }

  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  boost::intrusive_ptr<IJavaVariableType> pRetVal = nullptr;
  if (parsedType.returnType.At(0) != c_JavaTypeSpecifierVoid)
  {
    pRetVal = pVirtualMachineState->PopOperand();
  }

  pVirtualMachineState->PopOperand();
  if (nullptr != pRetVal)
  {
    pVirtualMachineState->PushOperand(pRetVal);
  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  return e_IncreaseCallStackDepth::No;
}

void BasicExecutionEngine::ExecuteOpCodeMonitorExit(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  auto pOperand = pVirtualMachineState->PopOperand();

  if (e_JavaVariableTypes::Object == pOperand->GetVariableType())
  {
    boost::intrusive_ptr<ObjectReference> pReference = boost::dynamic_pointer_cast<ObjectReference>(pOperand);
    if (nullptr == pReference)
    {
      throw InvalidStateException(__FUNCTION__ " - Expected object on operand stack.");
    }

    pReference->GetContainedObject()->MonitorExit(pVirtualMachineState->GetCurrentClassAndMethodName().ToUtf8String().c_str());
  }
  else if (e_JavaVariableTypes::Array == pOperand->GetVariableType())
  {
    boost::intrusive_ptr<ObjectReference> pReference = boost::dynamic_pointer_cast<ObjectReference>(pOperand);
    if (nullptr == pReference)
    {
      throw InvalidStateException(__FUNCTION__ " - Expected array on operand stack.");
    }

    pReference->GetContainedArray()->MonitorExit(pVirtualMachineState->GetCurrentClassAndMethodName().ToUtf8String().c_str());
  }
  else if (e_JavaVariableTypes::ClassReference == pOperand->GetVariableType())
  {
    boost::intrusive_ptr<JavaClassReference> pReference = boost::dynamic_pointer_cast<JavaClassReference>(pOperand);
    if (nullptr == pReference)
    {
      throw InvalidStateException(__FUNCTION__ " - Expected array on operand stack.");
    }

    pReference->GetClassFile()->MonitorExit(pVirtualMachineState->GetCurrentClassAndMethodName().ToUtf8String().c_str());
  }
  else
  {
    throw InvalidArgumentException(__FUNCTION__ " - Expected object or array on operand stack.");
  }
}

std::shared_ptr<MethodInfo> BasicExecutionEngine::IdentifyVirtualMethodToCall(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, std::shared_ptr<MethodInfo> pMethodInfo, boost::intrusive_ptr<ObjectReference> pObject)
{
#ifdef _DEBUG
  std::shared_ptr<MethodInfo> pOldMethod = pMethodInfo;
#endif // _DEBUG

  JavaString methodName = *(pMethodInfo->GetName());
  JavaString methodType = *(pMethodInfo->GetType());

  std::shared_ptr<JavaClass> pClass = pObject->GetContainedObject()->GetClass();
  std::shared_ptr<MethodInfo> pMethodToExecute = pClass->GetMethodByNameAndType(methodName, methodType);


  if (nullptr == pMethodToExecute)
  {
    std::shared_ptr<JavaClass> pSuperClass = pClass->GetSuperClass();
    while (nullptr != pSuperClass)
    {
      pMethodToExecute = pSuperClass->GetMethodByNameAndType(methodName, methodType);
      if (nullptr != pMethodToExecute)
      {
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
        if (pVirtualMachineState->HasUserCodeStarted())
        {
          GetLogger()->LogDebug("Method found on Superclass of %s : Found method %s with type %s on superclass %s.", pObject->GetContainedObject()->GetClass()->GetName()->ToUtf8String().c_str(), pMethodToExecute->GetName()->ToUtf8String().c_str(), pMethodToExecute->GetType()->ToUtf8String().c_str(), pMethodToExecute->GetClass()->GetName()->ToUtf8String().c_str());
        }
#endif // _DEBUG
        break;
      }

      pSuperClass = pSuperClass->GetSuperClass();
    }
  }

#ifdef _DEBUG
  if (nullptr == pMethodToExecute)
  {
    pVirtualMachineState->LogCallStack();
    pVirtualMachineState->LogOperandStack();
    BreakDebug("a", "a");
  }
#endif // _DEBUG

  return pMethodToExecute;
}

bool BasicExecutionEngine::DoesClassImplementInterface(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState,
  std::shared_ptr<JavaClass> pClass,
  boost::intrusive_ptr<JavaString> nameOfInterface)
{
  return HelperClasses::DoesClassImplementInterface(pVirtualMachineState, pClass, nameOfInterface);
}

e_IncreaseCallStackDepth BasicExecutionEngine::ExecuteOpCodeInvokeInterfaceMethod(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  ConstantPoolIndex methodIndex = ReadIndex(pVirtualMachineState);
  uint8_t count = ReadByteUnsigned(pVirtualMachineState);
  ReadByteUnsigned(pVirtualMachineState);

  std::shared_ptr<ConstantPoolEntry> pConstant = pVirtualMachineState->GetConstantFromCurrentClass(methodIndex);
  if (pConstant->GetType() != e_ConstantPoolEntryTypeInterfaceMethodReference)
  {
    throw InvalidStateException(__FUNCTION__ " - Expected Method reference.");
  }

  std::shared_ptr<ConstantPoolMethodReference> pMethodRef = pConstant->AsMethodReference();

  auto params = TypeParser::ParseMethodType(*pMethodRef->GetType());

  boost::intrusive_ptr<ObjectReference> pObject = boost::dynamic_pointer_cast<ObjectReference>(pVirtualMachineState->PeekOperandFromBack(static_cast<uint8_t>(params.parameters.size()) + 1));
  if (nullptr == pObject)
  {
#if defined (_DEBUG)
    pVirtualMachineState->LogCallStack();
    pVirtualMachineState->LogOperandStack();
    pVirtualMachineState->LogLocalVariables();
#endif

    throw InvalidStateException(__FUNCTION__ " - Expected object pointer to be valid.");
  }

  if (pObject->IsNull())
  {
#if defined (_DEBUG)
    pVirtualMachineState->LogCallStack();
    pVirtualMachineState->LogOperandStack();
    pVirtualMachineState->LogLocalVariables();
#endif

    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaNullPointerExceptionException);
    return e_IncreaseCallStackDepth::No;
  }

  std::shared_ptr<MethodInfo> pMethodInfo = pVirtualMachineState->ResolveMethodOnClass(pObject->GetContainedObject()->GetClass()->GetName(), pMethodRef.get());
  if (nullptr == pMethodInfo)
  {
#if defined (_DEBUG)
    pVirtualMachineState->LogCallStack();
    pVirtualMachineState->LogOperandStack();
    pVirtualMachineState->LogLocalVariables();
#endif

    GetLogger()->LogError("Could not resolve method (%s) on class: (%s).", pMethodInfo->GetName()->ToUtf8String().c_str(), pMethodInfo->GetClass()->GetName()->ToUtf8String().c_str());

    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaIncompatibleClassChangeErrorException);
    return e_IncreaseCallStackDepth::No;
  }

  std::vector<boost::intrusive_ptr<IJavaVariableType> > paramArray = pVirtualMachineState->PopulateParameterArrayFromOperandStack(pMethodInfo);

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Invoking Interface Method: %s.", pMethodInfo->GetName()->ToUtf8String().c_str());
  }
#endif // _DEBUG

  return ExecuteVirtualMethodInternal(pVirtualMachineState, pMethodInfo, pObject, paramArray, e_MethodAlreadyIdentified::Yes);
}

e_IncreaseCallStackDepth BasicExecutionEngine::ExecuteVirtualMethod(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, std::shared_ptr<ConstantPoolEntry> pConstant)
{
  std::shared_ptr<ConstantPoolMethodReference> pMethodRef = pConstant->AsMethodReference();
  std::shared_ptr<MethodInfo> pMethodInfo = pVirtualMachineState->ResolveMethodOnClass(pMethodRef->GetClassName(), pMethodRef.get());

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif

  if (nullptr == pMethodInfo)
  {
#if defined (_DEBUG)
    pVirtualMachineState->LogCallStack();
    pVirtualMachineState->LogOperandStack();
    pVirtualMachineState->LogLocalVariables();
#endif

    GetLogger()->LogError("Could not resolve method (%s) on class: (%s).", pMethodInfo->GetName()->ToUtf8String().c_str(), pMethodInfo->GetClass()->GetName()->ToUtf8String().c_str());
    throw InvalidStateException(__FUNCTION__ " - Could not resolve method on class.");
  }

  // TODO: Access Checks as per 5.4.3.1 (.3)

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Invoking Virtual Method: %s::%s.", pMethodInfo->GetClass()->GetName()->ToUtf8String().c_str(), pMethodInfo->GetName()->ToUtf8String().c_str());
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  std::vector<boost::intrusive_ptr<IJavaVariableType> > paramArray = pVirtualMachineState->PopulateParameterArrayFromOperandStack(pMethodInfo);

  if (paramArray[0]->GetVariableType() == e_JavaVariableTypes::Object)
  {
    return ExecuteVirtualMethodForObject(pVirtualMachineState, paramArray, pMethodInfo);
  }
  else if (paramArray[0]->GetVariableType() == e_JavaVariableTypes::Array)
  {
    ExecuteVirtualMethodForArray(pVirtualMachineState, paramArray, pMethodInfo);

    return e_IncreaseCallStackDepth::No;
  }
  else if (paramArray[0]->IsNull())
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaNullPointerExceptionException);
    return e_IncreaseCallStackDepth::No;
  }

  throw InvalidStateException(__FUNCTION__ " - Object or Array Expected.");
}

void BasicExecutionEngine::ExecuteVirtualMethodForArray(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, std::vector<boost::intrusive_ptr<IJavaVariableType> > paramArray, std::shared_ptr<MethodInfo> pMethodInfo)
{
  boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>(paramArray[0]);

  if (*pMethodInfo->GetClass()->GetName() != JavaString::FromCString("java/lang/Object"))
  {
    throw InvalidStateException(__FUNCTION__ " - Expected class name to be Object.");
  }

  if (*pMethodInfo->GetName() == JavaString::FromCString("clone"))
  {
    std::shared_ptr<IThreadManager> pThreadManager = GlobalCatalog::GetInstance().Get("ThreadManager");
    boost::intrusive_ptr<ObjectReference> pNewArray = pThreadManager->GetCurrentThreadState()->CreateArray(pArray->GetContainedArray()->GetContainedType(), pArray->GetContainedArray()->GetNumberOfElements());

    pNewArray->CloneOther(*pArray);

    pVirtualMachineState->PushOperand(pNewArray);
  }
#ifdef _DEBUG
  else
  {
    //BreakDebug( __FUNCTION__, "Need to implement this" );
    BreakDebug("a", "a");
  }
#endif // _DEBUG
}

e_IncreaseCallStackDepth BasicExecutionEngine::ExecuteVirtualMethodForObject(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, std::vector<boost::intrusive_ptr<IJavaVariableType> > paramArray, std::shared_ptr<MethodInfo> pMethodInfo)
{
  boost::intrusive_ptr<ObjectReference> pObject = boost::dynamic_pointer_cast<ObjectReference>(paramArray[0]);

  if (nullptr == pObject)
  {
#if defined (_DEBUG)
    pVirtualMachineState->LogCallStack();
    pVirtualMachineState->LogOperandStack();
    pVirtualMachineState->LogLocalVariables();
#endif

    throw InvalidStateException(__FUNCTION__ " - Expected object pointer to be valid.");
  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  return ExecuteVirtualMethodInternal(pVirtualMachineState, pMethodInfo, pObject, paramArray, e_MethodAlreadyIdentified::No);
}

e_IncreaseCallStackDepth BasicExecutionEngine::ExecuteVirtualMethodInternal(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, std::shared_ptr<MethodInfo>& pMethodInfo, boost::intrusive_ptr<ObjectReference> pObject, std::vector<boost::intrusive_ptr<IJavaVariableType> > paramArray, e_MethodAlreadyIdentified methodAlreadyIdentified)
{
  if (pMethodInfo->IsProtected())
  {
    // Finally, if the resolved method is protected, and it is a member of a superclass of the current class, and the method is not
    // declared in the same runtime package as the current class
    std::shared_ptr<JavaClass> pSuperClass = pVirtualMachineState->GetCurrentClass()->GetSuperClass();
    if (pSuperClass->GetMethodByName(*(pMethodInfo->GetName())))
    {
      if (pVirtualMachineState->GetCurrentClass()->GetPackageName() != pMethodInfo->GetPackageName())
      {
        // then the class of *objectref* must be either the current class or a subclass of the current class.
        if (!IsInstanceOf(pVirtualMachineState, pVirtualMachineState->GetCurrentClass()->GetName(), pObject->GetContainedObject()->GetClass()->GetName()))
        {
          throw InvalidStateException(__FUNCTION__ " - Expected object reference to be of the current class or a subclass.");
        }
      }
    }
  }

  if (pMethodInfo->IsStatic())
  {
    throw InvalidArgumentException(__FUNCTION__ " - Method to invoke was meant to be non-static.");
  }

  if (!pMethodInfo->IsSignaturePolymorphic())
  {
    if (e_MethodAlreadyIdentified::No == methodAlreadyIdentified)
    {
      pMethodInfo = IdentifyVirtualMethodToCall(pVirtualMachineState, pMethodInfo, pObject);
      if (nullptr == pMethodInfo)
      {
        HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaAbstractMethodErrorException);
        return e_IncreaseCallStackDepth::No;
      }
    }

    try
    {
      //std::shared_ptr<JavaClassFile> pFinalClass = pObject->GetClass();

      // The objectref must be followed on the operand stack by nargs argument values, where the number, type, and order of the values
      // must be consistent with the descriptor of the selected instance method.

      if (pMethodInfo->IsSynchronised())
      {
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
        if (pVirtualMachineState->HasUserCodeStarted())
        {
          GetLogger()->LogDebug("Virtual Method is synchronized: %s.", pMethodInfo->GetName()->ToUtf8String().c_str());
        }
#endif // _DEBUG

        pVirtualMachineState->PushMonitor(pObject->GetContainedObject()->MonitorEnter(pMethodInfo->GetFullName().ToUtf8String().c_str()));
      }

      if (!pMethodInfo->IsNative())
      {
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
        if (pVirtualMachineState->HasUserCodeStarted())
        {
          GetLogger()->LogDebug("Setting up %u local variables", pMethodInfo->GetCodeInfo()->GetLocalVariableArraySizeIncludingPassedParameters());
          pVirtualMachineState->LogOperandStack();
        }
#endif

        //If the method is not native, the nargs argument values and objectref are popped from the operand stack.
        pVirtualMachineState->PushState(*(pMethodInfo->GetClass()->GetName()), *(pMethodInfo->GetName()), *(pMethodInfo->GetType()), pMethodInfo);
        pVirtualMachineState->SetupLocalVariables(pMethodInfo, pObject, paramArray);

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
        if (pVirtualMachineState->HasUserCodeStarted())
        {
          pVirtualMachineState->LogLocalVariables();
        }
#endif // _DEBUG

        JVMX_ASSERT(*pMethodInfo->GetClass()->GetName() == *pObject->GetContainedObject()->GetClass()->GetName() || IsSuperClassOf(pVirtualMachineState, pMethodInfo->GetClass()->GetName(), pObject->GetContainedObject()->GetClass()->GetName()));

        pVirtualMachineState->SetCodeSegment(pMethodInfo->GetCodeInfo());

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
        if (pVirtualMachineState->HasUserCodeStarted())
        {
          GetLogger()->LogDebug("Executing Method (virtual/interface): %s::%s", pMethodInfo->GetClass()->GetName()->ToUtf8String().c_str(), pMethodInfo->GetName()->ToUtf8String().c_str());
        }
#endif // _DEBUG

        return e_IncreaseCallStackDepth::Yes;
      }
      else
      {
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
        if (pVirtualMachineState->HasUserCodeStarted())
        {
          GetLogger()->LogDebug("Method implementation is native: %s type: %s.", pMethodInfo->GetName()->ToUtf8String().c_str(), pMethodInfo->GetType()->ToUtf8String().c_str());
          GetLogger()->LogDebug("Setting up %u local variables", paramArray.size());
          pVirtualMachineState->LogOperandStack();
        }
#endif // _DEBUG

        TypeParser::ParsedMethodType type = TypeParser::ParseMethodType(*(pMethodInfo->GetType()));

        // We have to push the parameters back into the stack, because we will read them off again later as integers in the lead up to the native method.
        if (paramArray.size() > 1)
        {
          for (size_t ip = 1; ip < paramArray.size(); ++ip)
          {
            pVirtualMachineState->PushOperand(paramArray[ip]);
          }
        }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
        if (pVirtualMachineState->HasUserCodeStarted())
        {
          pVirtualMachineState->LogOperandStack();
          pVirtualMachineState->LogLocalVariables();
        }
#endif // _DEBUG

        std::shared_ptr<JavaNativeInterface> pJNI = pVirtualMachineState->GetJavaNativeInterface();
        //pVirtualMachineState->PushState( *( pMethodInfo->GetClass()->GetName() ), *( pMethodInfo->GetName() ), *( pMethodInfo->GetType() ), pMethodInfo );
        pJNI->ExecuteFunction(JavaNativeInterface::MakeJNINameWithoutArgumentDescriptor(pMethodInfo->GetClass()->GetName(), pMethodInfo->GetName()), pMethodInfo->GetType(), pObject);
        //pVirtualMachineState->PopState();

        if (pMethodInfo->IsSynchronised())
        {
          pVirtualMachineState->PopMonitor()->Unlock(pMethodInfo->GetFullName().ToUtf8String().c_str());
        }
        return e_IncreaseCallStackDepth::No;
      }
    }
    catch (std::exception& ex)
    {
      GetLogger()->LogDebug(__FUNCTION__ " - Exception Caught:\n\t%s", ex.what());

      pVirtualMachineState->PopState();
      pVirtualMachineState->ReleaseLocalVariables();

      throw;
    }
    catch (...)
    {
#ifdef _DEBUG
      GetLogger()->LogDebug(__FUNCTION__ " - Exception Caught.");
#endif // _DEBUG
      pVirtualMachineState->PopState();
      pVirtualMachineState->ReleaseLocalVariables();

      throw;
    }
  }
  else
  {
    throw NotImplementedException(__FUNCTION__ " - Not implemented yet.");
  }

  return e_IncreaseCallStackDepth::No;
}
