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


void BasicExecutionEngine::ExecuteOpCodeGetStatic(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  // The runtime constant pool item at that index must be a symbolic reference to a field
  std::shared_ptr<ConstantPoolEntry> fieldEntry = pVirtualMachineState->GetConstantFromCurrentClass(ReadIndex(pVirtualMachineState));
  if (e_ConstantPoolEntryTypeFieldReference != fieldEntry->GetType())
  {
    GetLogger()->LogError(__FUNCTION__ " - Could not resolve field reference for opcode: getstatic");
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaIncompatibleClassChangeErrorException);
    return;
  }

  const ConstantPoolFieldReference* pFieldRef = dynamic_cast<const ConstantPoolFieldReference*>(fieldEntry->GetValue());
  if (nullptr == pFieldRef)
  {
    throw TypeMismatchException(__FUNCTION__ " - Could not convert constant pool entry to field reference.");
  }

  JavaString targetClassClassName = *pFieldRef->GetClassName();

  try
  {
    // On successful resolution of the field, the class or interface that declared the resolved field is initialized if that class or
    // interface has not already been initialized.
    if (!pVirtualMachineState->IsClassInitialised(targetClassClassName))
    {
      pVirtualMachineState->InitialiseClass(targetClassClassName);
    }
  }
  catch (FileDoesNotExistException& ex)
  {
    GetLogger()->LogDebug(__FUNCTION__ " - File not found: %s", ex.what());
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaJavaClassNotFoundException);
    return;
  }

  JavaString referencedClassName = *pFieldRef->GetType();
  if (TypeParser::IsReferenceTypeDescriptor(referencedClassName) && !TypeParser::IsArrayTypeDescriptor(referencedClassName))
  {
    referencedClassName = TypeParser::ExtractClassNameFromReference(referencedClassName);
    // On successful resolution of the field, the class or interface that declared the resolved field is initialized if that class or
    // interface has not already been initialized.

    try
    {
      if (!pVirtualMachineState->IsClassInitialised(referencedClassName))
      {
        pVirtualMachineState->InitialiseClass(referencedClassName);
      }
    }
    catch (FileDoesNotExistException& ex)
    {
      GetLogger()->LogDebug(__FUNCTION__ " - File not found2: %s", ex.what());
      HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaJavaClassNotFoundException);
      return;
    }

  }

  // The value of the class or interface field is fetched and pushed onto the operand stack.
  std::shared_ptr<FieldInfo> pField = ResolveField(pVirtualMachineState, targetClassClassName, *pFieldRef->GetName());
  if (nullptr == pField)
  {
    // We would have already thrown a Java Exception
    return;
  }

  //   const CodeAttributeList &attributes = pField->GetAttributes( );
  //   const std::shared_ptr<JavaCodeAttribute> pValue = attributes.at( pField->GetConstantValueIndex( ) );
  //   AttributeConstantValue valueAttribute = pValue->ToConstantValue();

  if (!pField->IsStatic())
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaIncompatibleClassChangeErrorException);
  }

  //GetLogger()->LogDebug( "Getting Value of field: %s::%s", className.ToByteArray(), *pFieldRef->GetName( )->ToByteArray( ) );

  pVirtualMachineState->PushOperand(pField->GetStaticValue());
}

std::shared_ptr<FieldInfo> BasicExecutionEngine::ResolveField(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, const JavaString& className, const JavaString& fieldName)
{
  auto it = m_ResolvedFields.find(std::make_pair(className, fieldName));
  if (it != m_ResolvedFields.end())
  {
    return it->second;
  }

  std::shared_ptr<JavaClass> pClass = ResolveClass(pVirtualMachineState, className);
  if (nullptr == pClass)
  {
    GetLogger()->LogError(__FUNCTION__ " - Could not resolve Class %s.", className.ToUtf8String().c_str());

    // We would have already thrown a JavaException.
    return nullptr;
  }

  std::shared_ptr<FieldInfo> pResult = pClass->GetFieldByName(fieldName);
  if (nullptr == pResult)
  {
    std::shared_ptr<JavaClass> pSuperClass = pClass->GetSuperClass();
    while (nullptr != pSuperClass)
    {
      if (nullptr == pSuperClass)
      {
        GetLogger()->LogError(__FUNCTION__ " - Could not resolve Super Class %s.", pClass->GetSuperClass()->GetName()->ToUtf8String().c_str());

        // We would have already thrown a JavaException.
        return nullptr;
      }

      pResult = pSuperClass->GetFieldByName(fieldName);
      if (nullptr != pResult)
      {
        break;
      }

      pSuperClass = pSuperClass->GetSuperClass();
    }
  }

  if (nullptr != pResult)
  {
    m_ResolvedFields.insert(std::make_pair(std::make_pair(className, fieldName), pResult));
  }

  return pResult;
}

std::shared_ptr<JavaClass> BasicExecutionEngine::ResolveClass(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, const JavaString& className)
{
  return HelperClasses::ResolveClass(pVirtualMachineState, className);
}

void BasicExecutionEngine::ExecuteOpCodeNewMultiDimentionalArray(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  uint16_t index = ReadIndex(pVirtualMachineState);

  uint8_t dimentionCount = ReadByteUnsigned(pVirtualMachineState);
  JVMX_ASSERT(dimentionCount >= 1);

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif

#ifdef _DEBUG
  if (dimentionCount > 2)
  {
    // We have not tested with more than 2 dimentions yet.
    BreakDebug("a", "a");
  }
#endif // _DEBUG

  std::vector<int32_t> dimentionSizes;
  for (uint8_t i = 0; i < dimentionCount; ++i)
  {
    JVMX_ASSERT(e_JavaVariableTypes::Integer == pVirtualMachineState->PeekOperand()->GetVariableType());
    dimentionSizes.push_back(boost::dynamic_pointer_cast<JavaInteger>(pVirtualMachineState->PopOperand())->ToHostInt32());
  }

  std::shared_ptr<ConstantPoolEntry> pConstantPoolEntry = pVirtualMachineState->GetConstantFromCurrentClass(index);
  std::shared_ptr<ConstantPoolClassReference> pClassRef = pConstantPoolEntry->AsClassReferencePointer();

  e_JavaArrayTypes finalDimentionType = TypeParser::ExtractContainedTypeFromArrayTypeDescriptor(*pClassRef->GetClassName());

  if (1 == dimentionCount) // Just handle this edge case.
  {
    boost::intrusive_ptr<ObjectReference> pFirstDimention = pVirtualMachineState->CreateArray(finalDimentionType, dimentionSizes[0]);
    pVirtualMachineState->PushOperand(pFirstDimention);
    return;
  }

  boost::intrusive_ptr<ObjectReference> pFirstDimention = pVirtualMachineState->CreateArray(e_JavaArrayTypes::Reference, dimentionSizes[0]);

  int32_t currentDimention = 0;
  InitialiseDimention(dimentionSizes, pVirtualMachineState, pFirstDimention, dimentionCount, currentDimention, finalDimentionType);

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("New Multi Array: %s", pFirstDimention->ToString().ToUtf8String().c_str());
  }
#endif

  pVirtualMachineState->PushOperand(pFirstDimention);
}

void BasicExecutionEngine::InitialiseDimention(const std::vector<int32_t>& dimentionSizes, const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, boost::intrusive_ptr<ObjectReference> pFirstDimention, uint8_t dimentionCount, int32_t currentDimention, e_JavaArrayTypes finalDimentionType)
{
  e_JavaArrayTypes arrayTypeForCurrentDimention = e_JavaArrayTypes::Reference;
  int32_t currentDimentionSize = dimentionSizes[currentDimention];

  if (currentDimention == dimentionCount - 2)
  {
    arrayTypeForCurrentDimention = finalDimentionType;
  }

  for (int32_t j = 0; j < dimentionSizes[currentDimention]; ++j)
  {
    boost::intrusive_ptr<ObjectReference> pNextDim = pVirtualMachineState->CreateArray(arrayTypeForCurrentDimention, currentDimentionSize);
    pFirstDimention->GetContainedArray()->SetAt(j, pNextDim.get());

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
      GetLogger()->LogDebug("New inner Array: %s", pNextDim->ToString().ToUtf8String().c_str());
    }
#endif

    if (currentDimention < dimentionCount - 2)
    {
      InitialiseDimention(dimentionSizes, pVirtualMachineState, pNextDim, dimentionCount, currentDimention + 1, finalDimentionType);
    }
  }
}

void BasicExecutionEngine::ExecuteOpCodePutStatic(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
    pVirtualMachineState->LogCallStack();
  }
#endif // _DEBUG

  // The runtime constant pool item at that index must be a symbolic reference to a field
  std::shared_ptr<ConstantPoolFieldReference> pFieldEntry = pVirtualMachineState->GetConstantFromCurrentClass(ReadIndex(pVirtualMachineState))->AsFieldReferencePointer();

  //std::shared_ptr<FieldInfo> pFieldInfo = ResolveField( pVirtualMachineState->GetCurrentClassName(), *pFieldEntry->GetName() );
  std::shared_ptr<FieldInfo> pFieldInfo = ResolveField(pVirtualMachineState, *pFieldEntry->GetClassName(), *pFieldEntry->GetName());
  if (nullptr == pFieldInfo)
  {
    GetLogger()->LogError(__FUNCTION__ " - Could not resolve field reference for opcode: getstatic");
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaIncompatibleClassChangeErrorException);
    return;
  }

  if (!pVirtualMachineState->IsClassInitialised(*pFieldEntry->GetClassName()))
  {
    pVirtualMachineState->InitialiseClass(*pFieldEntry->GetClassName());
  }

  if (!pFieldInfo->IsStatic())
  {
    GetLogger()->LogError(__FUNCTION__ " - Throwing Java Exception because field is not static.");
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaIncompatibleClassChangeErrorException);
  }

  if (pFieldInfo->IsFinal() && !(pVirtualMachineState->GetCurrentMethodName() == c_ClassInitialisationMethodName))
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaIllegalAccessErrorException);
  }

  boost::intrusive_ptr<IJavaVariableType> pOperand = pVirtualMachineState->PopOperand();

  // Now Assign the Value

#ifdef _DEBUG
  ValidateStaticFieldType(pFieldEntry, pFieldInfo, pOperand);
#endif // _DEBUG

  pFieldInfo->SetStaticValue(pOperand);
}

void BasicExecutionEngine::ExecuteOpCodeNewArrayOfReference(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  ConstantPoolIndex index = ReadIndex(pVirtualMachineState);

  boost::intrusive_ptr< JavaInteger > pCount = boost::dynamic_pointer_cast<JavaInteger>(pVirtualMachineState->PopOperand());
  if (nullptr == pCount)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Expected an operand of type int.");
  }

  if (pCount->ToHostInt32() < 0)
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaNegativeArraySizeExceptionException);
    return;
  }

  //std::shared_ptr<JavaClass> pClass = ResolveClassFromIndex( index );

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Creating array of type reference");
  }
#endif

  boost::intrusive_ptr<ObjectReference> pArray = pVirtualMachineState->CreateArray(e_JavaArrayTypes::Reference, pCount->ToHostInt32());

  pVirtualMachineState->PushOperand(pArray);

  //pVirtualMachineState->PushOperand( new JavaArray( e_JavaArrayTypes::Reference, pCount->ToHostInt32() ) );
}

void BasicExecutionEngine::ExecuteOpCodeNew(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  std::shared_ptr<JavaClass> pClass = ResolveClassFromIndex(pVirtualMachineState, ReadIndex(pVirtualMachineState));

  if (nullptr == pClass)
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaInstantiationErrorException);
    throw InvalidStateException(__FUNCTION__ " - Could not resolve class from index.");
  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Creating object of type: %s", pClass->GetName()->ToUtf8String().c_str());
  }
#endif // defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)

  if (!pClass->IsInitialsed())
  {
    pVirtualMachineState->InitialiseClass(*pClass->GetName());
  }

  pVirtualMachineState->PushOperand(pVirtualMachineState->CreateObject(pClass));

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
}

std::shared_ptr<JavaClass> BasicExecutionEngine::ResolveClassFromIndex(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, ConstantPoolIndex index)
{
  std::shared_ptr<ConstantPoolClassReference> pClassRef = pVirtualMachineState->GetConstantFromCurrentClass(index)->AsClassReferencePointer();
  if (nullptr == pClassRef)
  {
    throw InvalidStateException(__FUNCTION__ " - Expected index to be a reference to a class/interface/array.");
  }

  return ResolveClass(pVirtualMachineState, *pClassRef->GetClassName());
}

bool BasicExecutionEngine::IsSuperClassOfCurrentClass(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, boost::intrusive_ptr<JavaString> pClassName) const
{
  return IsSuperClassOf(pVirtualMachineState, pClassName, pVirtualMachineState->GetCurrentClass()->GetName());
}

void BasicExecutionEngine::ExecuteOpCodeNewArray(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  boost::intrusive_ptr< JavaInteger > pCount = boost::dynamic_pointer_cast<JavaInteger>(pVirtualMachineState->PopOperand());
  if (nullptr == pCount)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Expected an operand of type int.");
  }

  if (pCount->ToHostInt32() < 0)
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaNegativeArraySizeExceptionException);
    return;
  }

  uint8_t type = ReadByteUnsigned(pVirtualMachineState);
  ValidateArrayType(type);

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Creating array of type %d", static_cast<int>(type));
  }
#endif

  boost::intrusive_ptr<ObjectReference> pArray = pVirtualMachineState->CreateArray(static_cast<e_JavaArrayTypes>(type), pCount->ToHostInt32());

  pVirtualMachineState->PushOperand(pArray);
}

void BasicExecutionEngine::ExecuteOpCodeGetField(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  std::shared_ptr<ConstantPoolFieldReference> pFieldRef = pVirtualMachineState->GetConstantFromCurrentClass(ReadIndex(pVirtualMachineState))->AsFieldReferencePointer();

  std::shared_ptr<FieldInfo> pFieldInfo = ResolveField(pVirtualMachineState, *pFieldRef->GetClassName(), *pFieldRef->GetName());

  if (nullptr == pFieldInfo)
  {
    throw InvalidStateException(__FUNCTION__ " - Could not find field.");
  }

  if (pFieldInfo->IsStatic())
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaIncompatibleClassChangeErrorException);
    return;
  }

  if (pVirtualMachineState->PeekOperand()->GetVariableType() == e_JavaVariableTypes::NullReference)
  {
#ifdef _DEBUG
    GetLogger()->LogDebug("Null reference trying to get field: %s.%s", pFieldRef->GetClassName()->ToUtf8String().c_str(), pFieldRef->GetName()->ToUtf8String().c_str());
#endif // _DEBUG

#if defined (_DEBUG)
    pVirtualMachineState->LogCallStack();
    pVirtualMachineState->LogOperandStack();
    pVirtualMachineState->LogLocalVariables();
#endif

    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaNullPointerExceptionException);
    return;
  }

  boost::intrusive_ptr<ObjectReference> pObject = boost::dynamic_pointer_cast<ObjectReference>(pVirtualMachineState->PopOperand());

  if (pFieldInfo->IsProtected())
  {
    // If the field is protected (§4.6), and it is a member of a superclass of the current class, and the field is not declared in the same runtime package
    // ( §5.3 ) as the current class, then the class of objectref must be either the current class or a subclass of the current class.

    // TODO: Implement this check.
  }

  auto pFieldValue = pObject->GetContainedObject()->GetFieldByName(*pFieldInfo->GetName());
  if (nullptr == pFieldValue)
  {
#ifdef _DEBUG
    __asm int 3;
#endif
    throw InvalidStateException(__FUNCTION__ " - Could not find expected field on object.");
  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Object (%s)", pObject->ToString().ToUtf8String().c_str());
    GetLogger()->LogDebug("Getting Field (%s) value (%s)", pFieldInfo->GetName()->ToUtf8String().c_str(), pFieldValue->ToString().ToUtf8String().c_str());
  }
#endif // _DEBUG
  pVirtualMachineState->PushOperand(pFieldValue);
}

void BasicExecutionEngine::ExecuteOpCodeArrayLength(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  boost::intrusive_ptr<IJavaVariableType> pOperand = pVirtualMachineState->PopOperand();

  if (pOperand->IsNull())
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaNullPointerExceptionException);
    return;
  }

  boost::intrusive_ptr<ObjectReference> pArrayRef = boost::dynamic_pointer_cast<ObjectReference>(pOperand);
  JVMX_ASSERT(e_JavaVariableTypes::Array == pArrayRef->GetVariableType());

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Getting length of array : %d", static_cast<int32_t>(pArrayRef->GetContainedArray()->GetNumberOfElements()));
  }
#endif // _DEBUG

  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(static_cast<int32_t>(pArrayRef->GetContainedArray()->GetNumberOfElements()))));
}

bool BasicExecutionEngine::IsReference(e_JavaVariableTypes type)
{
  return (e_JavaVariableTypes::Object == type) || (e_JavaVariableTypes::Array == type) || (e_JavaVariableTypes::ClassReference == type) || (e_JavaVariableTypes::NullReference == type) || (e_JavaVariableTypes::ReturnAddress == type);
}

void BasicExecutionEngine::ExecuteOpCodePutField(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug(__FUNCTION__ ":");
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  ConstantPoolIndex index = ReadIndex(pVirtualMachineState);

  std::shared_ptr<ConstantPoolFieldReference> pFieldReference = pVirtualMachineState->GetConstantFromCurrentClass(index)->AsFieldReferencePointer();
  if (nullptr == pFieldReference)
  {
    throw InvalidStateException(__FUNCTION__ " - Expected constant to be a field reference");
  }

  std::shared_ptr<FieldInfo> pFieldInfo = ResolveField(pVirtualMachineState, *pFieldReference->GetClassName(), *pFieldReference->GetName());

  boost::intrusive_ptr<IJavaVariableType> pValue = pVirtualMachineState->PopOperand();
  boost::intrusive_ptr<ObjectReference> pObject = boost::dynamic_pointer_cast<ObjectReference>(pVirtualMachineState->PopOperand());

  if (nullptr == pObject || pObject->IsNull())
  {
#ifdef _DEBUG
    pVirtualMachineState->LogCallStack();
    pVirtualMachineState->LogOperandStack();
    pVirtualMachineState->LogLocalVariables();
#endif // _DEBUG

    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaNullPointerExceptionException);
    return;
  }

  if (pFieldInfo->IsStatic())
  {
#ifdef _DEBUG
    pVirtualMachineState->LogCallStack();
    pVirtualMachineState->LogOperandStack();
    pVirtualMachineState->LogLocalVariables();
#endif // _DEBUG

    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaIncompatibleClassChangeErrorException);
    return;
  }

  if (pFieldInfo->IsFinal())
  {
    // If the field is final, it must be declared in the current class, and the instruction must occur in an instance
    if (*pFieldReference->GetClassName() != pVirtualMachineState->GetCurrentClassName() && c_InstanceInitialisationMethodName != pVirtualMachineState->GetCurrentMethodName())
    {
#ifdef _DEBUG
      pVirtualMachineState->LogCallStack();
      pVirtualMachineState->LogOperandStack();
      pVirtualMachineState->LogLocalVariables();
#endif // _DEBUG

      HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaIllegalAccessErrorException);
      return;
    }
  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Setting Field %s to %s", pFieldInfo->GetName()->ToUtf8String().c_str(), pValue->ToString().ToUtf8String().c_str());
  }
#endif // _DEBUG

  try
  {
    pObject->GetContainedObject()->SetField(*pFieldInfo->GetName(), pValue);
  }
  catch (...)
  {
#ifdef _DEBUG
    pVirtualMachineState->LogCallStack();
#endif // _DEBUG
    throw;
  }
}

bool BasicExecutionEngine::AreTypesCompatibile(boost::intrusive_ptr<JavaString> referenceType, boost::intrusive_ptr<JavaString> valueType)
{
  if (referenceType == valueType)
  {
    return true;
  }

  switch (referenceType->At(0))
  {
  case c_JavaTypeSpecifierBool:
  case c_JavaTypeSpecifierByte:
  case c_JavaTypeSpecifierChar:
  case c_JavaTypeSpecifierShort:
  case c_JavaTypeSpecifierInteger:
    return c_JavaTypeSpecifierInteger == valueType->At(0);
    break;

  case c_JavaTypeSpecifierLong:
    return c_JavaTypeSpecifierLong == valueType->At(0);
    break;

  case c_JavaTypeSpecifierFloat:
    return c_JavaTypeSpecifierFloat == valueType->At(0);
    break;

  case c_JavaTypeSpecifierDouble:
    return c_JavaTypeSpecifierDouble == valueType->At(0);
    break;

  case c_JavaTypeSpecifierArray:
    //TODO: This isn't quite right.
    return c_JavaTypeSpecifierArray == valueType->At(0);

  case c_JavaTypeSpecifierReference:
    //TODO: This isn't quite right.
    return c_JavaTypeSpecifierReference == valueType->At(0);
    break;

  default:
    throw UnsupportedTypeException(__FUNCTION__ " - Type not supported.");
    break;
  }

  return false;
}

bool BasicExecutionEngine::IsSuperClassOf(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, boost::intrusive_ptr<JavaString> pPossibleSuperClassName, boost::intrusive_ptr<JavaString> pDerivedClassName) const
{
  return HelperClasses::IsSuperClassOf(pVirtualMachineState, pPossibleSuperClassName, pDerivedClassName);
}

void BasicExecutionEngine::ExecuteOpCodeIsInstanceOf(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  //#if defined _DEBUG && defined(JVMX_LOG_VERBOSE)
  //  pVirtualMachineState->LogCallStack();
  //  pVirtualMachineState->LogOperandStack();
  //  pVirtualMachineState->LogLocalVariables();
  //#endif // _DEBUG

  boost::intrusive_ptr<IJavaVariableType> pOperand = pVirtualMachineState->PopOperand();
  ConstantPoolIndex index = ReadIndex(pVirtualMachineState);

  JVMX_ASSERT(pOperand->IsReferenceType());

  if (e_JavaVariableTypes::Object != pOperand->GetVariableType())
  {
    if (e_JavaVariableTypes::NullReference == pOperand->GetVariableType())
    {
      pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(0)));
      return;
    }

    if (e_JavaVariableTypes::ClassReference != pOperand->GetVariableType())
    {
      if (e_JavaVariableTypes::Array != pOperand->GetVariableType())
      {
        throw InvalidArgumentException(__FUNCTION__ " - Expected first operand to be an object or a null reference type.");
      }
    }
  }

  std::shared_ptr<JavaClass> pResolvedClass = ResolveClassFromIndex(pVirtualMachineState, index);
  if (nullptr == pResolvedClass)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Could not resolve class reference.");
  }

  if (e_JavaVariableTypes::Object == pOperand->GetVariableType())
  {
    boost::intrusive_ptr<ObjectReference> pObject = boost::dynamic_pointer_cast<ObjectReference>(pOperand);
    if (!pResolvedClass->IsInterface())
    {
      pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(IsInstanceOf(pVirtualMachineState, pResolvedClass->GetName(), pObject->GetContainedObject()->GetClass()->GetName()) ? 1 : 0)));
      return;
    }
    else
    {
      bool bFound = DoesClassImplementInterface(pVirtualMachineState, pObject->GetContainedObject()->GetClass(), pResolvedClass->GetName());
      pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(bFound ? 1 : 0)));
      return;
    }
  }
  else if (e_JavaVariableTypes::ClassReference == pOperand->GetVariableType())
  {
    // This should not happen.
    JVMX_ASSERT(false);
  }
  else if (e_JavaVariableTypes::Array == pOperand->GetVariableType())
  {
    // If S (pOperand) is the class of the object referred to by objectref 
    // and T (pResolvedClass) is the resolved class,

    boost::intrusive_ptr<ObjectReference> pOperandAsArray = boost::dynamic_pointer_cast<ObjectReference>(pOperand);
    if (nullptr == pOperandAsArray)
    {
      throw InvalidStateException(__FUNCTION__ " - Expected an array.");
    }

    if (pResolvedClass->IsInterface())
    {
      if (*pResolvedClass->GetName() == JavaString::FromCString("java/lang/Object") ||
        *pResolvedClass->GetName() == JavaString::FromCString("java/lang/Cloneable") ||
        *pResolvedClass->GetName() == JavaString::FromCString("java/io/Serializable")
        )
      {
        pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(1)));
        return;
      }
    }
    else if (pResolvedClass->IsArray())
    {
      // If T is an array type TC[], that is, an array of components of type TC,
      // then one of the following must be true:
      // - TC and SC are the same primitive type.
      auto pArray = pOperandAsArray->GetContainedArray();

      auto typeToMatch = TypeParser::ExtractContainedTypeFromArrayTypeDescriptor(*pResolvedClass->GetName());

      if (pArray->GetContainedType() == typeToMatch)
      {
        pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(1)));
        return;
      }

#ifdef _DEBUG
      BreakDebug("a", "a");
#endif // _DEBUG
      // TODO: 
      // - TC and SC are reference types, and type SC can be cast to TC by these runtime rules.
    }
    else
    {
      if (*pResolvedClass->GetName() == JavaString::FromCString(u"java/lang/Object"))
      {
        pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(1)));
        return;
      }
    }


    pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(0)));
    return;
  }

  JVMX_ASSERT(false);
  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(0)));
}

bool BasicExecutionEngine::IsInstanceOf(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, boost::intrusive_ptr<JavaString> pPossibleSuperClassName, boost::intrusive_ptr<JavaString> pDerivedClassName) const
{
  if (*pPossibleSuperClassName == *pDerivedClassName)
  {
    return true;
  }

  return IsSuperClassOf(pVirtualMachineState, pPossibleSuperClassName, pDerivedClassName);
}

void BasicExecutionEngine::ExecuteOpCodeCheckCast(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  boost::intrusive_ptr<IJavaVariableType> pOperand = pVirtualMachineState->PeekOperand();
  JVMX_ASSERT(pOperand->IsReferenceType());

  // NB: Need to do this in all cases, so we we can't skip if we find a null reference.
  ConstantPoolIndex classIndex = ReadIndex(pVirtualMachineState);

  if (e_JavaVariableTypes::NullReference == pOperand->GetVariableType())
  {
    return;
  }

  if (!pOperand->IsReferenceType())
  {
    throw InvalidArgumentException(__FUNCTION__ " - Expected first operand to be an a reference type.");
  }

  std::shared_ptr<ConstantPoolClassReference> pClassRef = pVirtualMachineState->GetConstantFromCurrentClass(classIndex)->AsClassReferencePointer();
  if (nullptr == pClassRef)
  {
    throw InvalidStateException(__FUNCTION__ " - Expected index to be a reference to a class/interface/array.");
  }

  std::shared_ptr<JavaClass> pResolvedClass = nullptr;
  bool isClassRefArray = false;

  JavaString className = *pClassRef->GetClassName();
  if (c_JavaTypeSpecifierArray == className.At(0))
  {
    while (c_JavaTypeSpecifierArray == className.At(0))
    {
      // Remove the ['s at the beginning
      className = className.SubString(1, className.GetLengthInCodePoints() - 1);
    }

    if (c_JavaTypeSpecifierReference == className.At(0))
    {
      // Remove the L and ; around the class Name
      className = className.SubString(1, className.GetLengthInCodePoints() - 2);
    }

    isClassRefArray = true;
  }
  else
  {
    pResolvedClass = ResolveClass(pVirtualMachineState, className);
    if (nullptr == pResolvedClass)
    {
      throw InvalidArgumentException(__FUNCTION__ " - Could not resolve class reference.");
    }
  }

  if (e_JavaVariableTypes::Object == pOperand->GetVariableType())
  {
    CheckCastForObjects(pVirtualMachineState, pOperand, pResolvedClass);
    return;
  }
  else if (e_JavaVariableTypes::ClassReference == pOperand->GetVariableType())
  {
    // This should not happen
    JVMX_ASSERT(false);
    //CheckCastForReferences( pOperand, pResolvedClass );
    return;
  }
  else if (e_JavaVariableTypes::Array == pOperand->GetVariableType())
  {
    CheckCastForArrays(pVirtualMachineState, pOperand, pResolvedClass, isClassRefArray, new JavaString(className));
    return;
  }

  return;
}

void BasicExecutionEngine::CheckCastForObjects(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, boost::intrusive_ptr<IJavaVariableType> pOperand, std::shared_ptr<JavaClass> pResolvedClass)
{
  boost::intrusive_ptr<ObjectReference> pOperandAsObject = boost::dynamic_pointer_cast<ObjectReference>(pOperand);
  if (!pResolvedClass->IsInterface())
  {
    if (!IsInstanceOf(pVirtualMachineState, pResolvedClass->GetName(), pOperandAsObject->GetContainedObject()->GetClass()->GetName()))
    {
      HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaClassCastException);
      return;
    }
  }
  else
  {
    if (DoesClassImplementInterface(pVirtualMachineState, pOperandAsObject->GetContainedObject()->GetClass(), pResolvedClass->GetName()))
    {
      return;
    }

    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaClassCastException);
    return;
  }
}

void BasicExecutionEngine::CheckCastForArrays(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, boost::intrusive_ptr<IJavaVariableType> pOperand, std::shared_ptr<JavaClass> pResolvedClass, bool isClassRefArray, boost::intrusive_ptr<JavaString> pClassName)
{
  boost::intrusive_ptr<ObjectReference> pOperandAsArray = boost::dynamic_pointer_cast<ObjectReference>(pOperand);
  if (nullptr != pResolvedClass)
  {
    if (!pResolvedClass->IsInterface())
    {
      if (*pResolvedClass->GetName() != JavaString::FromCString("java/lang/Object"))
      {
        HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaClassCastException);
        return;
      }
    }

    if (*pResolvedClass->GetName() != JavaString::FromCString("java/lang/Object") &&
      *pResolvedClass->GetName() != JavaString::FromCString("java/lang/Cloneable") &&
      *pResolvedClass->GetName() != JavaString::FromCString("java/io/Serializable"))
    {
      HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaClassCastException);
      return;
    }
  }
  else
  {
    // If T is an array type TC[], that is, an array of components of type TC, then one of the following must be true:
    // - TC and SC are the same primitive type.
    // - TC and SC are reference types, and type SC can be cast to TC by recursive application of these rules.

    JVMX_ASSERT(isClassRefArray);
    if (e_JavaArrayTypes::Reference != pOperandAsArray->GetContainedArray()->GetContainedType())
    {
      if (JavaArray::ConvertTypeFromChar(pClassName->At(0)) == pOperandAsArray->GetContainedArray()->GetContainedType())
      {
        return;
      }
    }
    else
    {
      if (0 == pOperandAsArray->GetContainedArray()->GetNumberOfElements() || e_JavaVariableTypes::NullReference == pOperandAsArray->GetContainedArray()->At(0)->GetVariableType())
      {
        // We don't know what's inside this array, but since it's empty it can still hold any object type.
        return;
      }

      const ObjectReference* pArrayElement = dynamic_cast<const ObjectReference*>(pOperandAsArray->GetContainedArray()->At(0));
      JVMX_ASSERT(nullptr != pArrayElement);
      if (IsInstanceOf(pVirtualMachineState, pClassName, pArrayElement->GetContainedObject()->GetClass()->GetName()))
      {
        return;
      }

      HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaClassCastException);
    }
  }
}

void BasicExecutionEngine::ValidateStaticFieldType(std::shared_ptr<ConstantPoolFieldReference> pFieldEntry, std::shared_ptr<FieldInfo> pFieldInfo, boost::intrusive_ptr<IJavaVariableType> pOperand)
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  GetLogger()->LogDebug("Field %s::%s descriptor is: %s", pFieldEntry->GetClassName()->ToUtf8String().c_str(), pFieldInfo->GetName()->ToUtf8String().c_str(), pFieldInfo->GetType()->ToUtf8String().c_str());
#endif // _DEBUG
  switch (pFieldInfo->GetType()->At(0))
  {
  case c_JavaTypeSpecifierByte:
  case c_JavaTypeSpecifierChar:
  case c_JavaTypeSpecifierInteger:
  case c_JavaTypeSpecifierShort:
  case c_JavaTypeSpecifierBool:
  {
    boost::intrusive_ptr< JavaInteger > pInteger = boost::dynamic_pointer_cast<JavaInteger>(pOperand);
    if (nullptr == pInteger)
    {
      throw InvalidArgumentException(__FUNCTION__ " - Expected operand to be integer type.");
    }
  }
  break;

  case c_JavaTypeSpecifierFloat:
  {
    boost::intrusive_ptr<JavaFloat> pFloat = boost::dynamic_pointer_cast<JavaFloat>(pOperand);
    if (nullptr == pFloat)
    {
      throw InvalidArgumentException(__FUNCTION__ " - Expected operand to be float type.");
    }
  }
  break;

  case c_JavaTypeSpecifierDouble:
  {
    boost::intrusive_ptr<JavaDouble> pDouble = boost::dynamic_pointer_cast<JavaDouble>(pOperand);
    if (nullptr == pDouble)
    {
      throw InvalidArgumentException(__FUNCTION__ " - Expected operand to be double type.");
    }
  }
  break;

  case c_JavaTypeSpecifierLong:
  {
    boost::intrusive_ptr<JavaLong> pLong = boost::dynamic_pointer_cast<JavaLong>(pOperand);
    if (nullptr == pLong)
    {
      throw InvalidArgumentException(__FUNCTION__ " - Expected operand to be long type.");
    }
  }
  break;

  case c_JavaTypeSpecifierReference:
  {
    /* boost::intrusive_ptr<ObjectReference> pNullReference = boost::dynamic_pointer_cast<JavaNullReference>(pOperand);
     if ( nullptr !=e )
     {
       break;
     }
    */
    if (pOperand->IsNull())
    {
      break;
    }

    boost::intrusive_ptr<ObjectReference> pReference = boost::dynamic_pointer_cast<ObjectReference>(pOperand);
    if (nullptr == pReference)
    {
      throw InvalidArgumentException(__FUNCTION__ " - Expected operand to be reference type.");
    }
  }
  break;

  case c_JavaTypeSpecifierArray:
  {
    boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>(pOperand);
    if (nullptr == pArray)
    {
      throw InvalidArgumentException(__FUNCTION__ " - Expected operand to be array type.");
    }
  }
  break;

  default:
    throw UnsupportedTypeException(__FUNCTION__ " - Unknown type found.");
    break;
  }
}

void BasicExecutionEngine::ValidateArrayType(uint8_t type)
{
  switch (static_cast<e_JavaArrayTypes>(type))
  {
  case e_JavaArrayTypes::Boolean:
  case e_JavaArrayTypes::Char:
  case e_JavaArrayTypes::Float:
  case e_JavaArrayTypes::Double:
  case e_JavaArrayTypes::Byte:
  case e_JavaArrayTypes::Short:
  case e_JavaArrayTypes::Integer:
  case e_JavaArrayTypes::Long:
    break;

  default:
    throw UnsupportedTypeException(__FUNCTION__ " - Creating array of unknown type");
    break;
  }
}
