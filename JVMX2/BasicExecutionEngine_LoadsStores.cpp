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


void BasicExecutionEngine::ExecuteOpCodeLoadReferenceFromLocalWithSpecifiedIndex(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  ExecuteOpCodeLoadReferenceFromLocalIndex(pVirtualMachineState, ReadByteUnsigned(pVirtualMachineState));
}

void BasicExecutionEngine::ExecuteOpCodeLoadReferenceFromLocalIndex(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, uint16_t index)
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogLocalVariables();
    pVirtualMachineState->LogOperandStack();
    GetLogger()->LogDebug("Loading reference: %s from local variable %d", pVirtualMachineState->GetLocalVariable(index)->ToString().ToUtf8String().c_str(), static_cast<int>(index));
  }
#endif // _DEBUG

  pVirtualMachineState->PushOperand(pVirtualMachineState->GetLocalVariable(index));
}

void BasicExecutionEngine::ExecuteOpCodePushNull(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  pVirtualMachineState->PushOperand(new ObjectReference(nullptr));
}

void BasicExecutionEngine::ExecuteOpCodeLoadDoubleFromArray(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  if (e_JavaVariableTypes::Integer != pVirtualMachineState->PeekOperand()->GetVariableType())
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected integer on the operand stack." ));
  }

  boost::intrusive_ptr< JavaInteger > pIndex = boost::dynamic_pointer_cast<JavaInteger>(pVirtualMachineState->PopOperand());

  if (e_JavaVariableTypes::Array != pVirtualMachineState->PeekOperand()->GetVariableType())
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected Array on the operand stack." ));
  }

  boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>(pVirtualMachineState->PopOperand());
  if (nullptr == pArray)
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected reference to JavaArray on operand stack." ));
  }

  if (pArray->IsNull())
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaNullPointerExceptionException);
    return;
  }

  if (pIndex->ToHostInt32() < 0 || pIndex->ToHostInt32() > static_cast<int32_t>(pArray->GetContainedArray()->GetNumberOfElements()))
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaArrayIndexOutOfBoundsException);
    return;
  }

  JavaDouble* pTempDouble = dynamic_cast<JavaDouble*>(pArray->GetContainedArray()->At(pIndex->ToHostInt32()));
  if (nullptr == pTempDouble)
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected double in array." ));
  }

  boost::intrusive_ptr<JavaDouble> pDouble = new JavaDouble(JavaDouble::FromHostDouble(pTempDouble->ToHostDouble()));
  pVirtualMachineState->PushOperand(pDouble);
}

void BasicExecutionEngine::ExecuteOpCodeWide(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
    pVirtualMachineState->LogLocalVariables();
  }
#endif // _DEBUG

  uint16_t opCode = ReadByteUnsigned(pVirtualMachineState);
  // The wide instruction is always followed by a 16-bit index, which is read as two bytes.

  uint8_t indexByte1 = ReadByteUnsigned(pVirtualMachineState);
  uint8_t indexByte2 = ReadByteUnsigned(pVirtualMachineState);
  uint16_t index = (indexByte1 << 8) | indexByte2;

  switch (opCode)
  {
  case (uint16_t)e_JavaOpCodes::LoadIntegerFromLocal:
    ExecuteOpCodeLoadIntegerFromLocal(pVirtualMachineState, index);
    break;
  case (uint16_t)e_JavaOpCodes::LoadLongFromLocal:
    ExecuteOpCodeLoadLongFromLocal(pVirtualMachineState, index);
    break;
  case (uint16_t)e_JavaOpCodes::LoadReferenceFromLocal:
    ExecuteOpCodeLoadReferenceFromLocalIndex(pVirtualMachineState, index);
    break;
  case (uint16_t)e_JavaOpCodes::StoreIntegerInLocal:
    ExecuteOpCodeStoreIntegerInLocal(pVirtualMachineState, index);
    break;
  case (uint16_t)e_JavaOpCodes::StoreLongInLocal:
    ExecuteOpCodeStoreLongInLocal(pVirtualMachineState, index);
    break;
  case (uint16_t)e_JavaOpCodes::StoreReferenceInLocal:
    ExecuteOpCodeStoreReferenceInLocal(pVirtualMachineState, index);
    break;
  case (uint16_t)e_JavaOpCodes::LoadFloatFromLocal:
    ExecuteOpCodeLoadFloatFromLocal(pVirtualMachineState, index);
    break;
  case (uint16_t)e_JavaOpCodes::StoreFloatInLocal:
    ExecuteOpCodeStoreFloatInLocal(pVirtualMachineState, index);
    break;
  case (uint16_t)e_JavaOpCodes::LoadDoubleFromLocal:
    ExecuteOpCodeLoadDoubleFromLocal(pVirtualMachineState, index);
    break;
  case (uint16_t)e_JavaOpCodes::StoreDoubleInLocal:
    ExecuteOpCodeStoreDoubleInLocal(pVirtualMachineState, index);
    break;

  case (uint16_t)e_JavaOpCodes::IncrementLocalVariable:
    uint8_t constByte1 = ReadByteUnsigned(pVirtualMachineState);
    uint8_t constByte2 = ReadByteUnsigned(pVirtualMachineState);
    uint16_t value = (constByte1 << 8) | constByte2;

    ExecuteOpCodeIncrementLocalVariable(pVirtualMachineState, index, value);

    break;

  }
}

void BasicExecutionEngine::ExecuteOpCodeStoreIntoDoubleArray(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined _DEBUG && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogCallStack();
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  boost::intrusive_ptr<IJavaVariableType> pValue = pVirtualMachineState->PopOperand();
  if (e_JavaVariableTypes::Double != pValue->GetVariableType())
  {
    throw InvalidArgumentException(JVMX_FUNCTION_MESSAGE( " - Expected third operand to be double type." ));
  }

  boost::intrusive_ptr< JavaInteger > pIndex = boost::dynamic_pointer_cast<JavaInteger>(pVirtualMachineState->PopOperand());
  if (nullptr == pIndex)
  {
    throw InvalidArgumentException(JVMX_FUNCTION_MESSAGE( " - Expected second operand to be integer type." ));
  }

  boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>(pVirtualMachineState->PopOperand());
  if (nullptr == pArray)
  {
    throw InvalidArgumentException(JVMX_FUNCTION_MESSAGE( " - Expected first operand to be array type." ));
  }

  if (pArray->IsNull())
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaNullPointerExceptionException);
  }

  if (pIndex->ToHostInt32() < 0 || static_cast<size_t>(pIndex->ToHostInt32()) > pArray->GetContainedArray()->GetNumberOfElements())
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaArrayIndexOutOfBoundsException);
  }

  pArray->GetContainedArray()->SetAt(*pIndex, pValue.get());
}

void BasicExecutionEngine::ExecuteOpCodeDuplicateTopOperandOrTwox1(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("**Before:");
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  auto pValue1 = pVirtualMachineState->PopOperand();
  auto pValue2 = pVirtualMachineState->PopOperand();

  if (IsCategoryOneType(pValue1) && IsCategoryOneType(pValue2))
  {
    auto pValue3 = pVirtualMachineState->PopOperand();
    JVMX_ASSERT(IsCategoryOneType(pValue3) && IsCategoryOneType(pValue2));

    pVirtualMachineState->PushOperand(pValue2);
    pVirtualMachineState->PushOperand(pValue1);

    pVirtualMachineState->PushOperand(pValue3);
    pVirtualMachineState->PushOperand(pValue2);
    pVirtualMachineState->PushOperand(pValue1);
  }
  else
  {
    JVMX_ASSERT(IsCategoryTwoType(pValue1) && IsCategoryOneType(pValue2));

    pVirtualMachineState->PushOperand(pValue1);
    pVirtualMachineState->PushOperand(pValue2);
    pVirtualMachineState->PushOperand(pValue1);
  }

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("**After:");
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

// Implement all four JVMS forms; the empty dispatch case left array assignments without the required duplicated values.
void BasicExecutionEngine::ExecuteOpCodeDuplicateTopOperandOrTwox2(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  const size_t stackSize = pVirtualMachineState->GetOperandStackSize();
  if (stackSize < 2)
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Insufficient operands for dup2_x2." ));
  }

  boost::intrusive_ptr<IJavaVariableType> pValue1 = pVirtualMachineState->PeekOperandFromBack(1);
  boost::intrusive_ptr<IJavaVariableType> pValue2 = pVirtualMachineState->PeekOperandFromBack(2);
  boost::intrusive_ptr<IJavaVariableType> pValue3;
  boost::intrusive_ptr<IJavaVariableType> pValue4;
  int form = 0;

  if (IsCategoryTwoType(pValue1))
  {
    if (IsCategoryTwoType(pValue2))
    {
      form = 4;
    }
    else if (IsCategoryOneType(pValue2) && stackSize >= 3)
    {
      pValue3 = pVirtualMachineState->PeekOperandFromBack(3);
      if (IsCategoryOneType(pValue3))
      {
        form = 3;
      }
    }
  }
  else if (IsCategoryOneType(pValue1) && IsCategoryOneType(pValue2) && stackSize >= 3)
  {
    pValue3 = pVirtualMachineState->PeekOperandFromBack(3);
    if (IsCategoryTwoType(pValue3))
    {
      form = 2;
    }
    else if (IsCategoryOneType(pValue3) && stackSize >= 4)
    {
      pValue4 = pVirtualMachineState->PeekOperandFromBack(4);
      if (IsCategoryOneType(pValue4))
      {
        form = 1;
      }
    }
  }

  if (0 == form)
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Operand types do not match a valid dup2_x2 form." ));
  }

  pVirtualMachineState->PopOperand();
  pVirtualMachineState->PopOperand();
  if (form == 1 || form == 3)
  {
    pVirtualMachineState->PopOperand();
  }
  if (form == 1)
  {
    pVirtualMachineState->PopOperand();
  }

  if (form == 1)
  {
    pVirtualMachineState->PushOperand(pValue2);
    pVirtualMachineState->PushOperand(pValue1);
    pVirtualMachineState->PushOperand(pValue4);
    pVirtualMachineState->PushOperand(pValue3);
    pVirtualMachineState->PushOperand(pValue2);
    pVirtualMachineState->PushOperand(pValue1);
  }
  else if (form == 2)
  {
    pVirtualMachineState->PushOperand(pValue2);
    pVirtualMachineState->PushOperand(pValue1);
    pVirtualMachineState->PushOperand(pValue3);
    pVirtualMachineState->PushOperand(pValue2);
    pVirtualMachineState->PushOperand(pValue1);
  }
  else if (form == 3)
  {
    pVirtualMachineState->PushOperand(pValue1);
    pVirtualMachineState->PushOperand(pValue3);
    pVirtualMachineState->PushOperand(pValue2);
    pVirtualMachineState->PushOperand(pValue1);
  }
  else
  {
    pVirtualMachineState->PushOperand(pValue1);
    pVirtualMachineState->PushOperand(pValue2);
    pVirtualMachineState->PushOperand(pValue1);
  }
}

void BasicExecutionEngine::ExecuteOpCodeConvertIntegerToShort(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  boost::intrusive_ptr< JavaInteger > pInteger1 = boost::dynamic_pointer_cast<JavaInteger>(pVirtualMachineState->PopOperand());
  if (nullptr == pInteger1)
  {
    throw InvalidArgumentException(JVMX_FUNCTION_MESSAGE( " - Expected first operand to be integer type." ));
  }

  boost::intrusive_ptr<JavaShort> pFinalShort = new JavaShort(JavaShort::FromHostInt16(static_cast<uint16_t>(pInteger1->ToHostInt32())));
  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromShort(*pFinalShort)));
}

void BasicExecutionEngine::ExecuteOpCodePushDouble(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, double value)
{
  pVirtualMachineState->PushOperand(new JavaDouble(JavaDouble::FromHostDouble(value)));
}

void BasicExecutionEngine::ExecuteOpCodeStoreFloatInLocal(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, uint16_t localVariableIndex)
{
  if (e_JavaVariableTypes::Float != pVirtualMachineState->PeekOperand()->GetVariableType())
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected float on the operand stack." ));
  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Storing float %s in local variable %d", pVirtualMachineState->PeekOperand()->ToString().ToUtf8String().c_str(), localVariableIndex);
  }
#endif // _DEBUG

  pVirtualMachineState->SetLocalVariable(localVariableIndex, pVirtualMachineState->PopOperand());
}

void BasicExecutionEngine::ExecuteOpCodeStoreFloatInLocalWithIndex(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  ExecuteOpCodeStoreFloatInLocal(pVirtualMachineState, ReadByteUnsigned(pVirtualMachineState));
}

void BasicExecutionEngine::ExecuteOpCodeLoadReferenceFromConstantPool(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  uint8_t indexByte = pVirtualMachineState->GetCodeSegmentStart()[pVirtualMachineState->GetProgramCounter()];
  pVirtualMachineState->AdvanceProgramCounter(1);

  std::shared_ptr<ConstantPoolEntry> pConstant = pVirtualMachineState->GetConstantFromCurrentClass(indexByte);

  if (e_ConstantPoolEntryTypeInteger == pConstant->GetType())
  {
    pVirtualMachineState->PushOperand(pConstant->AsInteger());
  }
  else if (e_ConstantPoolEntryTypeFloat == pConstant->GetType())
  {
    pVirtualMachineState->PushOperand(pConstant->AsFloat());
  }
  else if (e_ConstantPoolEntryTypeString == pConstant->GetType())
  {
    pVirtualMachineState->PushOperand(pConstant->AsString());
  }
  else if (e_ConstantPoolEntryTypeStringReference == pConstant->GetType())
  {
    std::shared_ptr<ConstantPoolStringReference> pStringRefRef = pConstant->AsStringReference();

    boost::intrusive_ptr<ObjectReference> pObject = GetFromStringPool(pStringRefRef);
    if (nullptr == pObject)
    {
      pObject = pVirtualMachineState->CreateStringObject(*pStringRefRef->GetStringValue());
      AddToStringPool(pStringRefRef, pObject);
    }

    pVirtualMachineState->PushOperand(pObject);
  }
  else if (e_ConstantPoolEntryTypeClassReference == pConstant->GetType())
  {
    std::shared_ptr<ConstantPoolClassReference> pClassRef = pConstant->AsClassReferencePointer();
    std::shared_ptr<JavaClass> pClassFile = ResolveClass(pVirtualMachineState, *pClassRef->GetClassName());

    if (nullptr == pClassFile)
    {
      throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Class was not resolved." ));
    }

    pVirtualMachineState->PushOperand(new JavaClassReference(pClassFile));
  }
  else if (e_ConstantPoolEntryTypeMethodHandle == pConstant->GetType() || e_ConstantPoolEntryTypeMethodType == pConstant->GetType())
  {
    throw NotImplementedException(JVMX_FUNCTION_MESSAGE( " - Not implemented yet." ));
  }
  else if (e_ConstantPoolEntryTypeMethodReference == pConstant->GetType())
  {
    //     std::shared_ptr<JavaClassFile> pClass = GetClassLibrary()->FindClass( *(pConstant->AsMethodReference()->GetName()) );
    //     pVirtualMachineState->PushOperand( pClass->GetMethodByNameAndType( *(pConstant->AsMethodReference( )->GetName( )), *(pConstant->AsMethodReference( )->GetType( )) ) );
    throw NotImplementedException(JVMX_FUNCTION_MESSAGE( " - Not implemented yet." ));
  }
  else
  {
    throw InvalidArgumentException(JVMX_FUNCTION_MESSAGE( " - Unexpected constant type." ));
  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Constant Value: %s", pVirtualMachineState->PeekOperand()->ToString().ToUtf8String().c_str());
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodePushInt(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, int value)
{
  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(value)));
}

void BasicExecutionEngine::ExecuteOpCodeDuplicateTopOperand(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Inside dup...");
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  auto pTop = pVirtualMachineState->PeekOperand();
  pVirtualMachineState->PushOperand(pTop);

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeDuplicateTopOperandx1(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  auto pValue1 = pVirtualMachineState->PopOperand();
  auto pValue2 = pVirtualMachineState->PopOperand();

  pVirtualMachineState->PushOperand(pValue1);
  pVirtualMachineState->PushOperand(pValue2);
  pVirtualMachineState->PushOperand(pValue1);

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeStoreLongInLocal(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, uint16_t localVariableIndex)
{
  if (e_JavaVariableTypes::Long != pVirtualMachineState->PeekOperand()->GetVariableType())
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected long on the operand stack." ));
  }

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Storing long %s in local variable %d", pVirtualMachineState->PeekOperand()->ToString().ToUtf8String().c_str(), localVariableIndex);
    GetLogger()->LogDebug("**Before:");
    pVirtualMachineState->LogLocalVariables();
  }
#endif

  pVirtualMachineState->SetLocalVariable(localVariableIndex, pVirtualMachineState->PopOperand());
  //pVirtualMachineState->SetLocalVariable( localVariableIndex + 1,new ObjectReference(nullptr) );

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("**After:");
    pVirtualMachineState->LogLocalVariables();
  }
#endif
}

void BasicExecutionEngine::ExecuteOpCodeStoreLongInLocalWithIndex(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  ExecuteOpCodeStoreLongInLocal(pVirtualMachineState, ReadByteUnsigned(pVirtualMachineState));
}

void BasicExecutionEngine::ExecuteOpCodePushIntImmediateByte(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  ExecuteOpCodePushInt(pVirtualMachineState, ReadByteSigned(pVirtualMachineState));
}

void BasicExecutionEngine::ExecuteOpCodePushFromConstantPoolWide(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  std::shared_ptr<ConstantPoolEntry> pEntry = pVirtualMachineState->GetConstantFromCurrentClass(ReadIndex(pVirtualMachineState));

  if (e_ConstantPoolEntryTypeInteger == pEntry->GetType())
  {
    pVirtualMachineState->PushOperand(pEntry->AsInteger());
  }
  else if (e_ConstantPoolEntryTypeFloat == pEntry->GetType())
  {
    pVirtualMachineState->PushOperand(pEntry->AsFloat());
  }
  else if (e_ConstantPoolEntryTypeString == pEntry->GetType())
  {
    pVirtualMachineState->PushOperand(pEntry->AsString());
  }
  else if (e_ConstantPoolEntryTypeStringReference == pEntry->GetType())
  {
    std::shared_ptr<ConstantPoolStringReference> pStringRefRef = pEntry->AsStringReference();

    boost::intrusive_ptr<ObjectReference> pObject = pVirtualMachineState->CreateStringObject(*pVirtualMachineState->GetConstantFromCurrentClass(pStringRefRef->ToConstantPoolIndex())->AsString());

    pVirtualMachineState->PushOperand(pObject);
  }
  else if (e_ConstantPoolEntryTypeClassReference == pEntry->GetType())
  {
    boost::intrusive_ptr<ObjectReference> pClassObject = pVirtualMachineState->CreateJavaLangClassFromClassName(pEntry->AsClassReferencePointer()->GetClassName());
    pVirtualMachineState->PushOperand(pClassObject);
  }
  else if (e_ConstantPoolEntryTypeMethodReference == pEntry->GetType())
  {
    throw NotImplementedException(JVMX_FUNCTION_MESSAGE( " - Not implemented yet." ));
  }
}

void BasicExecutionEngine::ExecuteOpCodeStoreIntoIntArray(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif

  boost::intrusive_ptr< IJavaVariableType > pOperand = pVirtualMachineState->PopOperand();

  //  boost::dynamic_pointer_cast<JavaInteger>(pOperand);
  if (!pOperand->IsIntegerCompatible())
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected integer on operand stack." ));
  }

  boost::intrusive_ptr< JavaInteger > pValue = TypeParser::UpCastToInteger(pOperand.get());

  boost::intrusive_ptr< JavaInteger > pIndex = boost::dynamic_pointer_cast<JavaInteger>(pVirtualMachineState->PopOperand());
  if (nullptr == pIndex)
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected integer on operand stack." ));
  }

  boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>(pVirtualMachineState->PopOperand());
  if (nullptr == pArray)
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected reference to JavaArray on operand stack." ));
  }

  if (pArray->IsNull())
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaNullPointerExceptionException);
  }

  if (pIndex->ToHostInt32() < 0 || static_cast<size_t>(pIndex->ToHostInt32()) > pArray->GetContainedArray()->GetNumberOfElements())
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaArrayIndexOutOfBoundsException);
  }

  pArray->GetContainedArray()->SetAt(*pIndex, *pValue);
}

void BasicExecutionEngine::ExecuteOpCodeStoreIntoLongArray(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif

  boost::intrusive_ptr< IJavaVariableType > pOperand = pVirtualMachineState->PopOperand();

  //  boost::dynamic_pointer_cast<JavaInteger>(pOperand);
  if (pOperand->GetVariableType() != e_JavaVariableTypes::Long)
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected long on operand stack." ));
  }

  boost::intrusive_ptr< JavaLong > pValue = boost::dynamic_pointer_cast<JavaLong>(pOperand);

  boost::intrusive_ptr< JavaInteger > pIndex = boost::dynamic_pointer_cast<JavaInteger>(pVirtualMachineState->PopOperand());
  if (nullptr == pIndex)
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected integer on operand stack." ));
  }

  boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>(pVirtualMachineState->PopOperand());
  if (nullptr == pArray)
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected reference to JavaArray on operand stack." ));
  }

  if (pArray->IsNull())
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaNullPointerExceptionException);
  }

  if (pIndex->ToHostInt32() < 0 || static_cast<size_t>(pIndex->ToHostInt32()) > pArray->GetContainedArray()->GetNumberOfElements())
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaArrayIndexOutOfBoundsException);
  }

  pArray->GetContainedArray()->SetAt(*pIndex, *pValue);
}

int BasicExecutionEngine::GetIntegerFromOperandStack(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState) const
{
  boost::intrusive_ptr<IJavaVariableType> pOperand = pVirtualMachineState->PopOperand();

  int result = 0;
  switch (pOperand->GetVariableType())
  {
  case e_JavaVariableTypes::Integer:
    result = boost::dynamic_pointer_cast<JavaInteger>(pOperand)->ToHostInt32();
    break;

  case e_JavaVariableTypes::Char:
    result = boost::dynamic_pointer_cast<JavaChar>(pOperand)->ToUInt16();
    break;

  case e_JavaVariableTypes::Bool:
    result = boost::dynamic_pointer_cast<JavaBool>(pOperand)->ToUint16();
    break;

  case e_JavaVariableTypes::Byte:
    result = boost::dynamic_pointer_cast<JavaByte>(pOperand)->ToHostInt8();
    break;

  case e_JavaVariableTypes::Short:
    result = boost::dynamic_pointer_cast<JavaShort>(pOperand)->ToHostInt16();
    break;

  default:
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Value on the operand stack was not of type integer." ));
  }

  return result;
}

int64_t BasicExecutionEngine::GetLongFromOperandStack(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  auto pOperand = pVirtualMachineState->PopOperand();

  if (e_JavaVariableTypes::Long == pOperand->GetVariableType())
  {
    return boost::dynamic_pointer_cast<JavaLong>(pOperand)->ToHostInt64();
  }

  if (!pOperand->IsIntegerCompatible())
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Value on the operand stack was not of type long or integer." ));
  }

  return TypeParser::UpCastToInteger(pOperand.get())->ToHostInt32();
}

void BasicExecutionEngine::ExecuteOpCodeStoreReferenceInLocalWithIndex(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  ExecuteOpCodeStoreReferenceInLocal(pVirtualMachineState, ReadByteUnsigned(pVirtualMachineState));
}

void BasicExecutionEngine::ExecuteOpCodeStoreReferenceInLocal(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, uint16_t localVariableIndex)
{
  e_JavaVariableTypes type = pVirtualMachineState->PeekOperand()->GetVariableType();
  if (!IsReference(type))
  {
#ifdef _DEBUG
    pVirtualMachineState->LogOperandStack();
    pVirtualMachineState->LogCallStack();
#endif // _DEBUG
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected reference on the operand stack." ));
  }

  pVirtualMachineState->SetLocalVariable(localVariableIndex, pVirtualMachineState->PopOperand());
}

void BasicExecutionEngine::ExecuteOpCodeStoreIntegerInLocal(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, uint16_t localVariableIndex)
{
  if (e_JavaVariableTypes::Integer != pVirtualMachineState->PeekOperand()->GetVariableType())
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected integer on the operand stack." ));
  }

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Storing Integer %s in local variable %d", pVirtualMachineState->PeekOperand()->ToString().ToUtf8String().c_str(), localVariableIndex);
  }
#endif // _DEBUG

  pVirtualMachineState->SetLocalVariable(localVariableIndex, pVirtualMachineState->PopOperand());
}

void BasicExecutionEngine::ExecuteOpCodeStoreIntegerInLocalWithIndex(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  ExecuteOpCodeStoreIntegerInLocal(pVirtualMachineState, ReadByteUnsigned(pVirtualMachineState));
}

void BasicExecutionEngine::ExecuteOpCodeLoadIntegerFromLocalWithIndex(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  ExecuteOpCodeLoadIntegerFromLocal(pVirtualMachineState, ReadByteUnsigned(pVirtualMachineState));
}

void BasicExecutionEngine::ExecuteOpCodeLoadIntegerFromLocal(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, uint16_t localVariableIndex)
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Inside iload_(%d)...", (int)localVariableIndex);
    pVirtualMachineState->LogLocalVariables();
  }
#endif

  auto pLocalValue = pVirtualMachineState->GetLocalVariable(localVariableIndex);
  if (!pLocalValue->IsIntegerCompatible())
  {
    pVirtualMachineState->LogLocalVariables();
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected integer in local variable." ));
  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Loading Integer %s from local variable %d", pVirtualMachineState->GetLocalVariable(localVariableIndex)->ToString().ToUtf8String().c_str(), localVariableIndex);
  }
#endif // _DEBUG

  pVirtualMachineState->PushOperand(TypeParser::UpCastToInteger(pVirtualMachineState->GetLocalVariable(localVariableIndex).get()));

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif
}

void BasicExecutionEngine::ExecuteOpCodeLoadReferenceFromArray(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  if (e_JavaVariableTypes::Integer != pVirtualMachineState->PeekOperand()->GetVariableType())
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected integer on the operand stack." ));
  }

  boost::intrusive_ptr< JavaInteger > pIndex = boost::dynamic_pointer_cast<JavaInteger>(pVirtualMachineState->PopOperand());

  if (e_JavaVariableTypes::Array != pVirtualMachineState->PeekOperand()->GetVariableType())
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected Array on the operand stack." ));
  }

  boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>(pVirtualMachineState->PopOperand());

  if (nullptr == pArray)
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected reference to JavaArray on operand stack." ));
  }

  if (pArray->IsNull())
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaNullPointerExceptionException);
    return;
  }

  if (pIndex->ToHostInt32() < 0 || pIndex->ToHostInt32() > static_cast<int32_t>(pArray->GetContainedArray()->GetNumberOfElements()))
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaArrayIndexOutOfBoundsException);
    return;
  }

  IJavaVariableType* pTempReference = pArray->GetContainedArray()->At(pIndex->ToHostInt32());
  boost::intrusive_ptr<ObjectReference> ref = new ObjectReference(*dynamic_cast<const ObjectReference*>(pTempReference));

  pVirtualMachineState->PushOperand(ref);
}

void BasicExecutionEngine::ExecuteOpCodeLoadCharacterFromArray(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  auto pair = LoadFromArrayInternal(pVirtualMachineState);

  if (pVirtualMachineState->HasExceptionOccurred())
  {
    return;
  }

  auto pArray = pair.first;
  auto index = pair.second;

  JVMX_ASSERT(pArray->GetContainedArray()->GetContainedType() == e_JavaArrayTypes::Char);

  const JavaChar* pChar = dynamic_cast<const JavaChar*>(pArray->GetContainedArray()->At(index));
  if (nullptr == pChar)
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Could not convert from array contained type to char." ));
  }

  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromChar(*pChar)));
}

std::pair< boost::intrusive_ptr<ObjectReference>, uint32_t> BasicExecutionEngine::LoadFromArrayInternal(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  std::pair< boost::intrusive_ptr<ObjectReference>, uint32_t> result;

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif

  if (e_JavaVariableTypes::Integer != pVirtualMachineState->PeekOperand()->GetVariableType())
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected integer on the operand stack." ));
  }

  boost::intrusive_ptr< JavaInteger > pIndex = boost::dynamic_pointer_cast<JavaInteger>(pVirtualMachineState->PopOperand());

  if (e_JavaVariableTypes::Array != pVirtualMachineState->PeekOperand()->GetVariableType())
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected Array on the operand stack." ));
  }

  boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>(pVirtualMachineState->PopOperand());

  if (nullptr == pArray)
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected reference to JavaArray on operand stack." ));
  }

  if (pArray->IsNull())
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaNullPointerExceptionException);
    return result;
  }

  if (pIndex->ToHostInt32() < 0 || pIndex->ToHostInt32() > static_cast<int32_t>(pArray->GetContainedArray()->GetNumberOfElements()))
  {
#ifdef _DEBUG
    pVirtualMachineState->LogCallStack();
#endif // _DEBUG

    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaArrayIndexOutOfBoundsException);
    return result;
  }

  result.first = pArray;
  result.second = pIndex->ToHostInt32();

  return result;
}

void BasicExecutionEngine::ExecuteOpCodeLoadShortFromArray(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  auto pair = LoadFromArrayInternal(pVirtualMachineState);
  auto pArray = pair.first;
  auto index = pair.second;

  if (pVirtualMachineState->HasExceptionOccurred())
  {
    return;
  }

  JVMX_ASSERT(pArray->GetContainedArray()->GetContainedType() == e_JavaArrayTypes::Short);

  const JavaShort* pShort = dynamic_cast<const JavaShort*>(pArray->GetContainedArray()->At(index));
  if (nullptr == pShort)
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Could not convert from array contained type to char." ));
  }

  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(pShort->ToHostInt16())));

}

void BasicExecutionEngine::ExecuteOpCodeLoadFloatFromLocalWithIndex(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  ExecuteOpCodeLoadFloatFromLocal(pVirtualMachineState, ReadByteUnsigned(pVirtualMachineState));
}

void BasicExecutionEngine::ExecuteOpCodeLoadFloatFromLocal(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, uint16_t localVariableIndex)
{
  if (e_JavaVariableTypes::Float != pVirtualMachineState->GetLocalVariable(localVariableIndex)->GetVariableType())
  {
    pVirtualMachineState->LogLocalVariables();
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected float in local variable." ));
  }

  pVirtualMachineState->PushOperand(pVirtualMachineState->GetLocalVariable(localVariableIndex));
}

void BasicExecutionEngine::ExecuteOpCodePushFloatConstant(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, float value)
{
  pVirtualMachineState->PushOperand(new JavaFloat(JavaFloat::FromHostFloat(value)));
}

void BasicExecutionEngine::ExecuteOpCodeStoreIntoCharArray(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  boost::intrusive_ptr< JavaInteger > pValue = nullptr;
  boost::intrusive_ptr<IJavaVariableType> pThirdArgument = pVirtualMachineState->PopOperand();
  if (e_JavaVariableTypes::Char == pThirdArgument->GetVariableType())
  {
    pValue = new JavaInteger(JavaInteger::FromHostInt32(boost::dynamic_pointer_cast<JavaChar>(pThirdArgument)->ToUInt16()));
  }
  else
  {
    pValue = boost::dynamic_pointer_cast<JavaInteger>(pThirdArgument);
    if (nullptr == pValue)
    {
      throw InvalidArgumentException(JVMX_FUNCTION_MESSAGE( " - Expected third operand to be integer type." ));
    }
  }

  StoreIntoArray(pVirtualMachineState, pValue);
}

void BasicExecutionEngine::ExecuteOpCodeStoreIntoShortArray(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  boost::intrusive_ptr< JavaInteger > pValue = nullptr;
  boost::intrusive_ptr<IJavaVariableType> pThirdArgument = pVirtualMachineState->PopOperand();
  if (e_JavaVariableTypes::Short == pThirdArgument->GetVariableType())
  {
    auto nativeValue = boost::dynamic_pointer_cast<JavaShort>(pThirdArgument)->ToHostInt16();
    pValue = new JavaInteger(JavaInteger::FromHostInt32(nativeValue));
  }
  else
  {
    pValue = boost::dynamic_pointer_cast<JavaInteger>(pThirdArgument);
    if (nullptr == pValue)
    {
      throw InvalidArgumentException(JVMX_FUNCTION_MESSAGE( " - Expected third operand to be integer type." ));
    }
  }

  StoreIntoArray(pVirtualMachineState, pValue);
}

void BasicExecutionEngine::ExecuteOpCodeStoreIntoByteArray(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  boost::intrusive_ptr< JavaInteger > pValue = nullptr;
  boost::intrusive_ptr<IJavaVariableType> pThirdArgument = pVirtualMachineState->PopOperand();
  if (e_JavaVariableTypes::Byte == pThirdArgument->GetVariableType())
  {
    uint8_t value = boost::dynamic_pointer_cast<JavaByte>(pThirdArgument)->ToHostInt8();
    pValue = new JavaInteger(JavaInteger::FromHostInt32(value));
  }
  else
  {
    pValue = boost::dynamic_pointer_cast<JavaInteger>(pThirdArgument);
    if (nullptr == pValue)
    {
      throw InvalidArgumentException(JVMX_FUNCTION_MESSAGE( " - Expected third operand to be integer type." ));
    }
  }

  StoreIntoArray(pVirtualMachineState, pValue);
}

void BasicExecutionEngine::ExecuteOpCodeStoreIntoReferenceArray(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined _DEBUG && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogCallStack();
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  boost::intrusive_ptr<IJavaVariableType> pValue = pVirtualMachineState->PopOperand();
  if (!pValue->IsReferenceType())
  {
    throw InvalidArgumentException(JVMX_FUNCTION_MESSAGE( " - Expected third operand to be reference type." ));
  }

  boost::intrusive_ptr< JavaInteger > pIndex = boost::dynamic_pointer_cast<JavaInteger>(pVirtualMachineState->PopOperand());
  if (nullptr == pIndex)
  {
    throw InvalidArgumentException(JVMX_FUNCTION_MESSAGE( " - Expected second operand to be integer type." ));
  }

  boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>(pVirtualMachineState->PopOperand());
  if (nullptr == pArray)
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected reference to JavaArray on operand stack." ));
  }

  if (pArray->IsNull())
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaNullPointerExceptionException);
    return;
  }

  if (pIndex->ToHostInt32() < 0 || static_cast<size_t>(pIndex->ToHostInt32()) > pArray->GetContainedArray()->GetNumberOfElements())
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaArrayIndexOutOfBoundsException);
  }

  pArray->GetContainedArray()->SetAt(*pIndex, pValue.get());
}

void BasicExecutionEngine::ExecuteOpCodeConvertIntegerToFloat(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  boost::intrusive_ptr< JavaInteger > pInteger1 = boost::dynamic_pointer_cast<JavaInteger>(pVirtualMachineState->PopOperand());
  if (nullptr == pInteger1)
  {
    throw InvalidArgumentException(JVMX_FUNCTION_MESSAGE( " - Expected first operand to be integer type." ));
  }

  pVirtualMachineState->PushOperand(new JavaFloat(JavaFloat::FromHostFloat(static_cast<float>(pInteger1->ToHostInt32()))));
}

void BasicExecutionEngine::ExecuteOpCodeConvertIntegerToDouble(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  boost::intrusive_ptr< JavaInteger > pInteger1 = boost::dynamic_pointer_cast<JavaInteger>(pVirtualMachineState->PopOperand());
  if (nullptr == pInteger1)
  {
    throw InvalidArgumentException(JVMX_FUNCTION_MESSAGE( " - Expected first operand to be integer type." ));
  }

  pVirtualMachineState->PushOperand(new JavaDouble(JavaDouble::FromHostDouble(static_cast<double>(pInteger1->ToHostInt32()))));
}

void BasicExecutionEngine::ExecuteOpCodeConvertFloatToInteger(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  boost::intrusive_ptr<JavaFloat> pFloat = boost::dynamic_pointer_cast<JavaFloat>(pVirtualMachineState->PopOperand());
  if (nullptr == pFloat)
  {
    throw InvalidArgumentException(JVMX_FUNCTION_MESSAGE( " - Expected first operand to be float type." ));
  }

  boost::intrusive_ptr< JavaInteger > result = new JavaInteger(JavaInteger::FromDefault());
  if (pFloat->IsNan())
  {
    result = new JavaInteger(JavaInteger::FromHostInt32(0));
  }
  else
  {
    result = new JavaInteger(JavaInteger::FromHostInt32(static_cast<int32_t>(pFloat->ToHostFloat())));
  }

  pVirtualMachineState->PushOperand(result);
}

void BasicExecutionEngine::ExecuteOpCodePopOperandStack(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  pVirtualMachineState->PopOperand();
}

void BasicExecutionEngine::ExecuteOpCodePopOperandStack_2(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Inside Pop2. Before:");
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  auto pOldOperand = pVirtualMachineState->PopOperand();
  if (IsCategoryOneType(pOldOperand))
  {
    JVMX_ASSERT(IsCategoryOneType(pVirtualMachineState->PeekOperand()));
    pVirtualMachineState->PopOperand();
  }

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Inside Pop2. After:");
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

}

void BasicExecutionEngine::ExecuteOpCodeIncrementLocalVariableWithIndex(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  uint8_t index = ReadByteUnsigned(pVirtualMachineState);
  int8_t constantValue = ReadByteSigned(pVirtualMachineState);

  ExecuteOpCodeIncrementLocalVariable(pVirtualMachineState, index, constantValue);
}

void BasicExecutionEngine::ExecuteOpCodeIncrementLocalVariable(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, uint16_t index, int16_t constantValue)
{
  boost::intrusive_ptr< JavaInteger > pIntValue = boost::dynamic_pointer_cast<JavaInteger>(pVirtualMachineState->GetLocalVariable(index));
  if (nullptr == pIntValue)
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected integer in local variable" ));
  }

  int32_t finalValue = pIntValue->ToHostInt32() + constantValue;

#ifdef _DEBUG
  if (finalValue < pIntValue->ToHostInt32() && constantValue > 0)
  {
    JVMX_ASSERT(false);
  }
#endif // _DEBUG

  pVirtualMachineState->SetLocalVariable(index, new JavaInteger(JavaInteger::FromHostInt32(finalValue)));
}

void BasicExecutionEngine::ExecuteOpCodeConvertIntegerToChar(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  boost::intrusive_ptr< JavaInteger > pInteger1 = boost::dynamic_pointer_cast<JavaInteger>(pVirtualMachineState->PopOperand());
  if (nullptr == pInteger1)
  {
    throw InvalidArgumentException(JVMX_FUNCTION_MESSAGE( " - Expected first operand to be integer type." ));
  }

  uint16_t characterValue = static_cast<uint16_t>(pInteger1->ToHostInt32());

  //boost::intrusive_ptr<JavaChar> pFinalChar = new JavaChar( JavaChar::FromUInt16( characterValue & 0x00FF ) );
  boost::intrusive_ptr<JavaChar> pFinalChar = new JavaChar(JavaChar::FromUInt16(characterValue & 0xFFFF));
  /*
  #ifdef _DEBUG
    if ( pFinalChar->ToUInt16() > UCHAR_MAX )
    {
      _asm int 3;
    }
  #endif // _DEBUG
  */
  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromChar(*pFinalChar)));
}

void BasicExecutionEngine::ExecuteOpCodePushShortConstant(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int16_t value = ReadShort(pVirtualMachineState);
  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromShort(JavaShort::FromHostInt16(value))));
}

void BasicExecutionEngine::ExecuteOpCodePushLong(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, int64_t value)
{
  pVirtualMachineState->PushOperand(new JavaLong(JavaLong::FromHostInt64(value)));
}

void BasicExecutionEngine::ExecuteOpCodeLoadLongFromLocalWithIndex(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  ExecuteOpCodeLoadLongFromLocal(pVirtualMachineState, ReadByteUnsigned(pVirtualMachineState));
}

void BasicExecutionEngine::ExecuteOpCodeLoadLongFromLocal(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, uint16_t localVariableIndex)
{
#if defined (_DEBUG) && defined (JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogLocalVariables();
  }
#endif // _DEBUG) && defined (JVMX_LOG_VERBOSE)

  boost::intrusive_ptr<IJavaVariableType> pLocalVariable = pVirtualMachineState->GetLocalVariable(localVariableIndex);
  boost::intrusive_ptr<JavaLong> pLongValue = nullptr;

  if (e_JavaVariableTypes::Long == pLocalVariable->GetVariableType())
  {
    pLongValue = boost::dynamic_pointer_cast<JavaLong>(pVirtualMachineState->GetLocalVariable(localVariableIndex));
  }
  else if (pLocalVariable->IsIntegerCompatible())
  {
    boost::intrusive_ptr<JavaInteger> pIntValue = TypeParser::UpCastToInteger(pLocalVariable.get());
    pLongValue = new JavaLong(JavaLong::FromHostInt64(pIntValue->ToHostInt32()));
  }
  else
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected long in local variable." ));
  }

#if defined(_DEBUG) && defined (JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Loading Long %s from local variable %d", pLongValue->ToString().ToUtf8String().c_str(), localVariableIndex);
  }
#endif // _DEBUG
  pVirtualMachineState->PushOperand(pLongValue);
}

void BasicExecutionEngine::ExecuteOpCodeLoadByteOrBooleanFromArray(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  if (e_JavaVariableTypes::Integer != pVirtualMachineState->PeekOperand()->GetVariableType())
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected integer on the operand stack." ));
  }

  boost::intrusive_ptr< JavaInteger > pIndex = boost::dynamic_pointer_cast<JavaInteger>(pVirtualMachineState->PopOperand());
  if (e_JavaVariableTypes::Array != pVirtualMachineState->PeekOperand()->GetVariableType())
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected Array on the operand stack." ));
  }

  boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>(pVirtualMachineState->PopOperand());
  if (nullptr == pArray)
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected reference to JavaArray on operand stack." ));
  }

  if (pArray->IsNull())
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaNullPointerExceptionException);
    return;
  }

  if (pIndex->ToHostInt32() < 0 || pIndex->ToHostInt32() > static_cast<int32_t>(pArray->GetContainedArray()->GetNumberOfElements()))
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaArrayIndexOutOfBoundsException);
    return;
  }

  if (pArray->GetContainedArray()->GetContainedType() != e_JavaArrayTypes::Boolean && pArray->GetContainedArray()->GetContainedType() != e_JavaArrayTypes::Byte)
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected boolean or byte in the array." ));
  }

  IJavaVariableType* pArrayValueAtIndex = pArray->GetContainedArray()->At(pIndex->ToHostInt32());
  JVMX_ASSERT(pArrayValueAtIndex->IsIntegerCompatible());

  if (e_JavaVariableTypes::Bool == pArrayValueAtIndex->GetVariableType())
  {
    JavaBool* pValueAsBool = dynamic_cast<JavaBool*>(pArrayValueAtIndex);
    boost::intrusive_ptr<JavaInteger> valueAsInt = new JavaInteger(JavaInteger::FromHostInt32(pValueAsBool->ToBool() ? 1 : 0));

    pVirtualMachineState->PushOperand(valueAsInt);
  }
  else
  {
    JavaByte* pValueAsByte = dynamic_cast<JavaByte*>(pArrayValueAtIndex);
    boost::intrusive_ptr<JavaInteger> valueAsInt = new JavaInteger(JavaInteger::FromHostInt32(static_cast<int32_t>(pValueAsByte->ToHostInt8())));

    pVirtualMachineState->PushOperand(valueAsInt);
  }
}

void BasicExecutionEngine::ExecuteOpCodeConvertIntegerToByte(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  boost::intrusive_ptr< JavaInteger > pInteger1 = boost::dynamic_pointer_cast<JavaInteger>(pVirtualMachineState->PopOperand());
  if (nullptr == pInteger1)
  {
    throw InvalidArgumentException(JVMX_FUNCTION_MESSAGE( " - Expected first operand to be integer type." ));
  }

  int32_t value = ConvertIntegerToByte(pInteger1);

  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(value)));
}

void BasicExecutionEngine::StoreIntoArray(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, boost::intrusive_ptr< JavaInteger > pValue)
{
  boost::intrusive_ptr< JavaInteger > pIndex = boost::dynamic_pointer_cast<JavaInteger>(pVirtualMachineState->PopOperand());
  if (nullptr == pIndex)
  {
    throw InvalidArgumentException(JVMX_FUNCTION_MESSAGE( " - Expected second operand to be integer type." ));
  }

  boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>(pVirtualMachineState->PopOperand());
  if (nullptr == pArray)
  {
    throw InvalidArgumentException(JVMX_FUNCTION_MESSAGE( " - Expected first operand to be array type." ));
  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug(JVMX_FUNCTION_MESSAGE( " - Storing value %s in index %d" ), (const char*)(pValue->ToString().ToUtf8String().c_str()), pIndex->ToHostInt32());
  }
#endif // _DEBUG

  if (pArray->IsNull())
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaNullPointerExceptionException);
  }

  if (pIndex->ToHostInt32() < 0 || static_cast<size_t>(pIndex->ToHostInt32()) > pArray->GetContainedArray()->GetNumberOfElements())
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaArrayIndexOutOfBoundsException);
  }

  pArray->GetContainedArray()->SetAt(*pIndex, *pValue);
}

int32_t BasicExecutionEngine::ConvertIntegerToByte(boost::intrusive_ptr< JavaInteger > pInteger1)
{
  int8_t byte = static_cast<int8_t>(pInteger1->ToHostInt32() & 0xFF);
  return byte;
}

void BasicExecutionEngine::ExecuteOpCodePushDoubleOrLongFromConstantPoolWide(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  std::shared_ptr<ConstantPoolEntry> pEntry = pVirtualMachineState->GetConstantFromCurrentClass(ReadIndex(pVirtualMachineState));

  if (e_ConstantPoolEntryTypeDouble == pEntry->GetType())
  {
    pVirtualMachineState->PushOperand(pEntry->AsDouble());
  }
  else if (e_ConstantPoolEntryTypeLong == pEntry->GetType())
  {
    pVirtualMachineState->PushOperand(pEntry->AsLong());
  }
  else
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected Long or Double constant at the index." ));
  }
}

void BasicExecutionEngine::ExecuteOpCodeConvertLongToInteger(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int64_t value1 = GetLongFromOperandStack(pVirtualMachineState);

#ifdef _DEBUG
  if (value1 < 0)
  {
    BreakDebug("a", "a"); // AI warned of an issue with negative numbers. "If value1 is negative, this will produce a large positive number, not the correct sign-extended value."
  }
#endif

  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32((int32_t)value1)));
}

void BasicExecutionEngine::ExecuteOpCodeConvertIntegerToLong(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int32_t value1 = GetIntegerFromOperandStack(pVirtualMachineState);

  //pVirtualMachineState->PushOperand( new JavaLong( JavaLong::FromHostInt64( value1 & 0xFFFFFFFF ) ) );
  pVirtualMachineState->PushOperand(new JavaLong(JavaLong::FromHostInt64(value1)));
}

void BasicExecutionEngine::ExecuteOpCodeConvertLongToDouble(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int64_t value1 = GetLongFromOperandStack(pVirtualMachineState);

  pVirtualMachineState->PushOperand(new JavaDouble(JavaDouble::FromHostInt64(value1)));
}

void BasicExecutionEngine::ExecuteOpCodeStoreDoubleInLocal(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, uint16_t localVariableIndex)
{
  if (e_JavaVariableTypes::Double != pVirtualMachineState->PeekOperand()->GetVariableType())
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected double on the operand stack." ));
  }

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Storing Double %s in local variable %d", pVirtualMachineState->PeekOperand()->ToString().ToUtf8String().c_str(), localVariableIndex);
  }
#endif // _DEBUG

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("**Before:");
    pVirtualMachineState->LogLocalVariables();
  }
#endif

  pVirtualMachineState->SetLocalVariable(localVariableIndex, pVirtualMachineState->PopOperand());
  pVirtualMachineState->SetLocalVariable(localVariableIndex + 1, new ObjectReference(nullptr));

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("**After:");
    pVirtualMachineState->LogLocalVariables();
  }
#endif
}

void BasicExecutionEngine::ExecuteOpCodeStoreDoubleInLocalWithIndex(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  ExecuteOpCodeStoreDoubleInLocal(pVirtualMachineState, ReadByteUnsigned(pVirtualMachineState));
}

void BasicExecutionEngine::ExecuteOpCodeLoadDoubleFromLocalWithIndex(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  ExecuteOpCodeLoadDoubleFromLocal(pVirtualMachineState, ReadByteUnsigned(pVirtualMachineState));
}

void BasicExecutionEngine::ExecuteOpCodeLoadDoubleFromLocal(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, uint16_t localVariableIndex)
{
  if (e_JavaVariableTypes::Double != pVirtualMachineState->GetLocalVariable(localVariableIndex)->GetVariableType())
  {
    pVirtualMachineState->LogLocalVariables();
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected Double in local variable." ));
  }

  pVirtualMachineState->PushOperand(pVirtualMachineState->GetLocalVariable(localVariableIndex));
}

void BasicExecutionEngine::ExecuteOpCodeDuplicateTopOperandOrTwo(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  auto pPeekValue = pVirtualMachineState->PeekOperand();
  if (IsCategoryTwoType(pPeekValue))
  {
    pVirtualMachineState->PushOperand(pPeekValue);
  }
  else
  {
    auto pValue2 = pVirtualMachineState->PeekOperand();
    pVirtualMachineState->PopOperand();

    auto pValue1 = pVirtualMachineState->PeekOperand();
    pVirtualMachineState->PushOperand(pValue2);

    pVirtualMachineState->PushOperand(pValue1);
    pVirtualMachineState->PushOperand(pValue2);
  }

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeLoadIntegerFromArray(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif

  if (e_JavaVariableTypes::Integer != pVirtualMachineState->PeekOperand()->GetVariableType())
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected integer on the operand stack." ));
  }

  boost::intrusive_ptr< JavaInteger > pIndex = boost::dynamic_pointer_cast<JavaInteger>(pVirtualMachineState->PopOperand());

#if 0
  GetLogger()->LogDebug("Before");
  pVirtualMachineState->LogCallStack();
  auto operand = pVirtualMachineState->PeekOperand();
  auto type = operand->GetVariableType();
  GetLogger()->LogDebug("After");
#endif

  if (e_JavaVariableTypes::Array != pVirtualMachineState->PeekOperand()->GetVariableType())
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected Array on the operand stack." ));
  }

  boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>(pVirtualMachineState->PopOperand());

  if (nullptr == pArray)
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected reference to JavaArray on operand stack." ));
  }

  if (pArray->IsNull())
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaNullPointerExceptionException);
    return;
  }

  if (pIndex->ToHostInt32() < 0 || pIndex->ToHostInt32() > static_cast<int32_t>(pArray->GetContainedArray()->GetNumberOfElements()))
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaArrayIndexOutOfBoundsException);
    return;
  }

  JVMX_ASSERT(pArray->GetContainedArray()->GetContainedType() == e_JavaArrayTypes::Integer);

  const JavaInteger* pResult = dynamic_cast<const JavaInteger*>(pArray->GetContainedArray()->At(pIndex->ToHostInt32()));
  if (nullptr == pResult)
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Could not convert from array contained type to integer." ));
  }

  pVirtualMachineState->PushOperand(new JavaInteger(*pResult));
}

void BasicExecutionEngine::ExecuteOpCodeLoadLongFromArray(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  auto pair = LoadFromArrayInternal(pVirtualMachineState);

  if (pVirtualMachineState->HasExceptionOccurred())
  {
    return;
  }

  auto pArray = pair.first;
  auto index = pair.second;

  JVMX_ASSERT(pArray->GetContainedArray()->GetContainedType() == e_JavaArrayTypes::Long);

  const JavaLong* pLong = dynamic_cast<const JavaLong*>(pArray->GetContainedArray()->At(index));
  if (nullptr == pLong)
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Could not convert from array contained type to char." ));
  }

  pVirtualMachineState->PushOperand(new JavaLong(*pLong));
}

bool BasicExecutionEngine::IsCategoryTwoType(boost::intrusive_ptr<IJavaVariableType> pValue)
{
  return e_JavaVariableTypes::Long == pValue->GetVariableType() || e_JavaVariableTypes::Double == pValue->GetVariableType();
}

bool BasicExecutionEngine::IsCategoryOneType(boost::intrusive_ptr<IJavaVariableType> pValue)
{
  return e_JavaVariableTypes::Long != pValue->GetVariableType() && e_JavaVariableTypes::Double != pValue->GetVariableType();
}

void BasicExecutionEngine::ExecuteOpCodeConvertDoubleToInt(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  boost::intrusive_ptr<JavaDouble> pDouble = boost::dynamic_pointer_cast<JavaDouble>(pVirtualMachineState->PopOperand());
  if (nullptr == pDouble)
  {
    throw InvalidArgumentException(JVMX_FUNCTION_MESSAGE( " - Expected first operand to be float type." ));
  }

  boost::intrusive_ptr< JavaInteger > result = new JavaInteger(JavaInteger::FromDefault());
  if (pDouble->IsNan())
  {
    result = new JavaInteger(JavaInteger::FromHostInt32(0));
  }
  else
  {
    result = new JavaInteger(JavaInteger::FromHostInt32(static_cast<int32_t>(pDouble->ToHostDouble())));
  }

  pVirtualMachineState->PushOperand(result);
}

void BasicExecutionEngine::ExecuteOpCodeConvertDoubleToFloat(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  boost::intrusive_ptr< JavaDouble > pDouble1 = boost::dynamic_pointer_cast<JavaDouble>(pVirtualMachineState->PopOperand());
  if (nullptr == pDouble1)
  {
    throw InvalidArgumentException(JVMX_FUNCTION_MESSAGE( " - Expected first operand to be double type." ));
  }

  pVirtualMachineState->PushOperand(new JavaFloat(JavaFloat::FromHostDouble(static_cast<double>(pDouble1->ToHostDouble()))));
}

void BasicExecutionEngine::ExecuteOpCodeConvertFloatToDouble(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  boost::intrusive_ptr< JavaFloat > pFloat1 = boost::dynamic_pointer_cast<JavaFloat>(pVirtualMachineState->PopOperand());
  if (nullptr == pFloat1)
  {
    throw InvalidArgumentException(JVMX_FUNCTION_MESSAGE( " - Expected first operand to be float type." ));
  }

  pVirtualMachineState->PushOperand(new JavaDouble(JavaDouble::FromHostDouble(static_cast<double>(pFloat1->ToHostFloat()))));
}

void BasicExecutionEngine::ExecuteOpCodeLoadFloatFromArray(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  if (e_JavaVariableTypes::Integer != pVirtualMachineState->PeekOperand()->GetVariableType())
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected integer on the operand stack." ));
  }

  boost::intrusive_ptr< JavaInteger > pIndex = boost::dynamic_pointer_cast<JavaInteger>(pVirtualMachineState->PopOperand());

  if (e_JavaVariableTypes::Array != pVirtualMachineState->PeekOperand()->GetVariableType())
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected Array on the operand stack." ));
  }

  boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>(pVirtualMachineState->PopOperand());
  if (nullptr == pArray)
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected reference to JavaArray on operand stack." ));
  }

  if (pArray->IsNull())
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaNullPointerExceptionException);
    return;
  }

  if (pIndex->ToHostInt32() < 0 || pIndex->ToHostInt32() > static_cast<int32_t>(pArray->GetContainedArray()->GetNumberOfElements()))
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaArrayIndexOutOfBoundsException);
    return;
  }

  JavaFloat* pTempFloat = dynamic_cast<JavaFloat*>(pArray->GetContainedArray()->At(pIndex->ToHostInt32()));
  if (nullptr == pTempFloat)
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected float in array." ));
  }

  boost::intrusive_ptr<JavaFloat> pFloat = new JavaFloat(JavaFloat::FromHostFloat(pTempFloat->ToHostFloat()));
  pVirtualMachineState->PushOperand(pFloat);
}

void BasicExecutionEngine::ExecuteOpCodeStoreIntoFloatArray(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined _DEBUG && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogCallStack();
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  boost::intrusive_ptr<IJavaVariableType> pValue = pVirtualMachineState->PopOperand();
  if (e_JavaVariableTypes::Float != pValue->GetVariableType())
  {
    throw InvalidArgumentException(JVMX_FUNCTION_MESSAGE( " - Expected third operand to be float type." ));
  }

  boost::intrusive_ptr< JavaInteger > pIndex = boost::dynamic_pointer_cast<JavaInteger>(pVirtualMachineState->PopOperand());
  if (nullptr == pIndex)
  {
    throw InvalidArgumentException(JVMX_FUNCTION_MESSAGE( " - Expected second operand to be integer type." ));
  }

  boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>(pVirtualMachineState->PopOperand());
  if (nullptr == pArray)
  {
    throw InvalidArgumentException(JVMX_FUNCTION_MESSAGE( " - Expected first operand to be array type." ));
  }

  if (pArray->IsNull())
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaNullPointerExceptionException);
  }

  if (pIndex->ToHostInt32() < 0 || static_cast<size_t>(pIndex->ToHostInt32()) > pArray->GetContainedArray()->GetNumberOfElements())
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaArrayIndexOutOfBoundsException);
  }

  pArray->GetContainedArray()->SetAt(*pIndex, pValue.get());
}

void BasicExecutionEngine::ExecuteOpCodeDuplicateTopOperandx2(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("**Before:");
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  auto pValue1 = pVirtualMachineState->PopOperand();
  auto pValue2 = pVirtualMachineState->PopOperand();

  if (IsCategoryOneType(pValue2))
  {
    auto pValue3 = pVirtualMachineState->PopOperand();
    JVMX_ASSERT(IsCategoryOneType(pValue3) && IsCategoryOneType(pValue2));

    pVirtualMachineState->PushOperand(pValue1);
    pVirtualMachineState->PushOperand(pValue3);
    pVirtualMachineState->PushOperand(pValue2);
    pVirtualMachineState->PushOperand(pValue1);
  }
  else
  {
    JVMX_ASSERT(IsCategoryOneType(pValue1) && IsCategoryTwoType(pValue2));

    pVirtualMachineState->PushOperand(pValue1);
    pVirtualMachineState->PushOperand(pValue2);
    pVirtualMachineState->PushOperand(pValue1);
  }

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("**After:");
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeConvertLongToFloat(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int64_t value1 = GetLongFromOperandStack(pVirtualMachineState);
  pVirtualMachineState->PushOperand(new JavaFloat(JavaFloat::FromHostFloat(static_cast<float>(value1))));
}

void BasicExecutionEngine::ExecuteOpCodeConvertFloatToLong(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  if (e_JavaVariableTypes::Float != pVirtualMachineState->PeekOperand()->GetVariableType())
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected float on the operand stack." ));
  }

  boost::intrusive_ptr<JavaFloat> pFloat = boost::dynamic_pointer_cast<JavaFloat>(pVirtualMachineState->PopOperand());
  int64_t result = static_cast<int64_t>(pFloat->ToHostFloat());
  pVirtualMachineState->PushOperand(new JavaLong(JavaLong::FromHostInt64(result)));
}

void BasicExecutionEngine::ExecuteOpCodeConvertDoubleToLong(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  if (e_JavaVariableTypes::Double != pVirtualMachineState->PeekOperand()->GetVariableType())
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Expected double on the operand stack." ));
  }

  boost::intrusive_ptr<JavaDouble> pDouble = boost::dynamic_pointer_cast<JavaDouble>(pVirtualMachineState->PopOperand());
  int64_t result = static_cast<int64_t>(pDouble->ToHostDouble());
  pVirtualMachineState->PushOperand(new JavaLong(JavaLong::FromHostInt64(result)));
}

void BasicExecutionEngine::ExecuteOpCodeSwapOperands(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  auto val2 = pVirtualMachineState->PopOperand();
  auto val1 = pVirtualMachineState->PopOperand();
  pVirtualMachineState->PushOperand(val2);
  pVirtualMachineState->PushOperand(val1);
}
