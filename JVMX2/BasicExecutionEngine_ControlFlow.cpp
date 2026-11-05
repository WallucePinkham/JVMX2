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


void BasicExecutionEngine::ExecuteOpCodeReturnVoid(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#ifdef _DEBUG
  const size_t debugSize = pVirtualMachineState->GetOperandStackSize();
#endif // _DEBUG

  if (pVirtualMachineState->GetCurrentMethodInfo()->IsSynchronised())
  {
    pVirtualMachineState->PopMonitor()->Unlock(pVirtualMachineState->GetCurrentMethodInfo()->GetFullName().ToUtf8String().c_str());
  }

  pVirtualMachineState->PopState();
  pVirtualMachineState->ReleaseLocalVariables();

#ifdef _DEBUG
  JVMX_ASSERT(pVirtualMachineState->GetOperandStackSize() == debugSize);
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeLookupSwitch(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  intptr_t startingAddress = pVirtualMachineState->GetProgramCounter() - 1; // -1 for the size of the switch instruction.

  //intptr_t paddingLength = 4 - ( pVirtualMachineState->GetProgramCounter() % 4 );
  intptr_t paddingLength = (4 - (pVirtualMachineState->GetProgramCounter() % 4)) % 4;

  JVMX_ASSERT(paddingLength < 4 && paddingLength >= 0);

  for (int i = 0; i < paddingLength; ++i)
  {
    uint8_t readByte = ReadByteUnsigned(pVirtualMachineState);
    readByte += 0;
  }

  JVMX_ASSERT((pVirtualMachineState->GetProgramCounter() % 4) == 0);

  const int32_t deflt = Read32BitOffset(pVirtualMachineState);
  const int32_t npairs = Read32BitOffset(pVirtualMachineState);

  JVMX_ASSERT(npairs >= 0);

  std::vector<std::pair<int32_t, int32_t>> jumpTable;
  for (int i = 0; i < npairs; ++i)
  {
    int32_t match = Read32BitOffset(pVirtualMachineState);
    int32_t offset = Read32BitOffset(pVirtualMachineState);

    jumpTable.push_back(std::make_pair(match, offset));
  }

  boost::intrusive_ptr< JavaInteger > pKey = boost::dynamic_pointer_cast<JavaInteger>(pVirtualMachineState->PopOperand());

  intptr_t targetAddress = 0;
  for (int i = 0; i < npairs; ++i)
  {
    if (jumpTable[i].first == pKey->ToHostInt32())
    {
      targetAddress = startingAddress + jumpTable[i].second;
      break;
    }
  }

  if (0 == targetAddress)
  {
    targetAddress = startingAddress + deflt;
  }

  pVirtualMachineState->AdvanceProgramCounter(targetAddress - pVirtualMachineState->GetProgramCounter());
}

void BasicExecutionEngine::ExecutedOpBranchIfEquals(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int16_t offset = ReadOffset(pVirtualMachineState);

  if (GetIntegerFromOperandStack(pVirtualMachineState) == 0)
  {
    AdjustProgramCounterByOffset(pVirtualMachineState, offset);
  }
}

void BasicExecutionEngine::ExecutedOpCodeBranchIfNotEquals(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int16_t offset = ReadOffset(pVirtualMachineState);

  if (GetIntegerFromOperandStack(pVirtualMachineState) != 0)
  {
    AdjustProgramCounterByOffset(pVirtualMachineState, offset);
  }
}

void BasicExecutionEngine::ExecutedOpCodeBranchIfLessThan(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int16_t offset = ReadOffset(pVirtualMachineState);

  if (GetIntegerFromOperandStack(pVirtualMachineState) < 0)
  {
    AdjustProgramCounterByOffset(pVirtualMachineState, offset);
  }
}

void BasicExecutionEngine::ExecutedOpCodeBranchIfGreaterThan(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int16_t offset = ReadOffset(pVirtualMachineState);

  if (GetIntegerFromOperandStack(pVirtualMachineState) > 0)
  {
    AdjustProgramCounterByOffset(pVirtualMachineState, offset);
  }
}

void BasicExecutionEngine::ExecutedOpCodeBranchIfGreaterThanOrEqual(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int16_t offset = ReadOffset(pVirtualMachineState);

  if (GetIntegerFromOperandStack(pVirtualMachineState) >= 0)
  {
    AdjustProgramCounterByOffset(pVirtualMachineState, offset);
  }
}

void BasicExecutionEngine::ExecutedOpCodeBranchIfLessThanOrEqual(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int16_t offset = ReadOffset(pVirtualMachineState);

  if (GetIntegerFromOperandStack(pVirtualMachineState) <= 0)
  {
    AdjustProgramCounterByOffset(pVirtualMachineState, offset);
  }
}

void BasicExecutionEngine::ExecutedOpBranchIfIntegerEquals(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int16_t offset = ReadOffset(pVirtualMachineState);

  int32_t value2 = GetIntegerFromOperandStack(pVirtualMachineState);
  int32_t value1 = GetIntegerFromOperandStack(pVirtualMachineState);

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("%s - Comparing integer values %ld and %ld and jumping to offset %d if they are equal.", pVirtualMachineState->GetCurrentClassAndMethodName().ToUtf8String().c_str(), value1, value2, offset);
  }
#endif // _DEBUG

  if (value1 == value2)
  {
    AdjustProgramCounterByOffset(pVirtualMachineState, offset);
  }
}

void BasicExecutionEngine::ExecutedOpBranchIfIntegerNotEquals(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int16_t offset = ReadOffset(pVirtualMachineState);

  int32_t value2 = GetIntegerFromOperandStack(pVirtualMachineState);
  int32_t value1 = GetIntegerFromOperandStack(pVirtualMachineState);

  if (value1 != value2)
  {
    AdjustProgramCounterByOffset(pVirtualMachineState, offset);
  }
}

void BasicExecutionEngine::ExecutedOpBranchIfIntegerLessThan(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int16_t offset = ReadOffset(pVirtualMachineState);

  int32_t value2 = GetIntegerFromOperandStack(pVirtualMachineState);
  int32_t value1 = GetIntegerFromOperandStack(pVirtualMachineState);

  if (value1 < value2)
  {
    AdjustProgramCounterByOffset(pVirtualMachineState, offset);
  }
}

void BasicExecutionEngine::ExecutedOpBranchIfIntegerGreatherThanOrEqual(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int16_t offset = ReadOffset(pVirtualMachineState);

  int32_t value2 = GetIntegerFromOperandStack(pVirtualMachineState);
  int32_t value1 = GetIntegerFromOperandStack(pVirtualMachineState);

  if (value1 >= value2)
  {
    AdjustProgramCounterByOffset(pVirtualMachineState, offset);
  }
}

void BasicExecutionEngine::ExecutedOpBranchIfIntegerGreatherThan(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int16_t offset = ReadOffset(pVirtualMachineState);

  int32_t value2 = GetIntegerFromOperandStack(pVirtualMachineState);
  int32_t value1 = GetIntegerFromOperandStack(pVirtualMachineState);

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("%s - Comparing integer values %ld and %ld and jumping to offset %d if %ld > %ld.", pVirtualMachineState->GetCurrentClassAndMethodName().ToUtf8String().c_str(), value1, value2, offset, value1, value2);
  }
#endif // _DEBUG


  if (value1 > value2)
  {
    AdjustProgramCounterByOffset(pVirtualMachineState, offset);
  }
}

void BasicExecutionEngine::ExecutedOpBranchIfIntegerLessThanOrEqual(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int16_t offset = ReadOffset(pVirtualMachineState);

  int32_t value2 = GetIntegerFromOperandStack(pVirtualMachineState);
  int32_t value1 = GetIntegerFromOperandStack(pVirtualMachineState);

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("%s - Comparing integer values %ld and %ld and jumping to offset %d if %ld <= %ld.", pVirtualMachineState->GetCurrentClassAndMethodName().ToUtf8String().c_str(), value1, value2, offset, value1, value2);
  }
#endif // _DEBUG

  if (value1 <= value2)
  {
    AdjustProgramCounterByOffset(pVirtualMachineState, offset);
  }
}

void BasicExecutionEngine::ExecuteOpCodeGoto(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  AdjustProgramCounterByOffset(pVirtualMachineState, ReadOffset(pVirtualMachineState));
}

void BasicExecutionEngine::AdjustProgramCounterByOffset(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, int16_t offset)
{
  pVirtualMachineState->AdvanceProgramCounter(offset - (sizeof(int16_t) + sizeof(int8_t)));
}

void BasicExecutionEngine::ExecuteOpCodeReturnReference(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#ifdef _DEBUG
  const size_t debugSize = pVirtualMachineState->GetOperandStackSize();
#endif // _DEBUG

  boost::intrusive_ptr<IJavaVariableType> pReference = pVirtualMachineState->PopOperand();
  if (!IsReference(pReference->GetVariableType()))
  {
    throw InvalidStateException(__FUNCTION__ " - Value on the operand stack was not of type reference.");
  }

  std::shared_ptr<MethodInfo> pMethodInfo = pVirtualMachineState->GetCurrentMethodInfo();
  if (pMethodInfo->IsSynchronised())
  {
    pVirtualMachineState->PopMonitor()->Unlock(pMethodInfo->GetFullName().ToUtf8String().c_str());
  }

  pVirtualMachineState->PopState();
  pVirtualMachineState->ReleaseLocalVariables();

  pVirtualMachineState->PushOperand(pReference);

#ifdef _DEBUG
  JVMX_ASSERT(pVirtualMachineState->GetOperandStackSize() == debugSize);
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeReturnInteger(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#ifdef _DEBUG
  const size_t debugSize = pVirtualMachineState->GetOperandStackSize();
#endif // _DEBUG

  std::shared_ptr<MethodInfo> pMethodInfo = pVirtualMachineState->GetCurrentMethodInfo();
  if (pMethodInfo->IsSynchronised())
  {
    pVirtualMachineState->PopMonitor()->Unlock(pMethodInfo->GetFullName().ToUtf8String().c_str());
  }

  if (pVirtualMachineState->HasExceptionOccurred())
  {
    return;
  }

  boost::intrusive_ptr<IJavaVariableType> pReference = pVirtualMachineState->PopOperand();
  if (pReference->GetVariableType() != e_JavaVariableTypes::Integer)
  {
    pReference = TypeParser::UpCastToInteger(pReference.get());
    if (nullptr == pReference)
    {
      throw InvalidStateException(__FUNCTION__ " - Value on the operand stack was not of type integer or convertible to integer.");
    }
  }

  pVirtualMachineState->PopState();
  pVirtualMachineState->ReleaseLocalVariables();

  pVirtualMachineState->PushOperand(pReference);

#ifdef _DEBUG
  JVMX_ASSERT(pVirtualMachineState->GetOperandStackSize() == debugSize);
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeBranchIfNotNull(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int16_t offset = ReadOffset(pVirtualMachineState);

  boost::intrusive_ptr<IJavaVariableType> pReference = pVirtualMachineState->PopOperand();
  if (!pReference->IsNull())
  {
    AdjustProgramCounterByOffset(pVirtualMachineState, offset);
  }
}

void BasicExecutionEngine::ExecuteOpCodeTableSwitch(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  intptr_t startingAddress = pVirtualMachineState->GetProgramCounter() - 1; // -1 for the size of the switch instruction.

  intptr_t paddingLength = (4 - (pVirtualMachineState->GetProgramCounter() % 4)) % 4;

  JVMX_ASSERT(paddingLength < 4 && paddingLength >= 0);

  for (int i = 0; i < paddingLength; ++i)
  {
    uint8_t readByte = ReadByteUnsigned(pVirtualMachineState);
    readByte += 0;
  }

  JVMX_ASSERT((pVirtualMachineState->GetProgramCounter() % 4) == 0);

  int32_t deflt = Read32BitOffset(pVirtualMachineState);
  int32_t low = Read32BitOffset(pVirtualMachineState);
  int32_t high = Read32BitOffset(pVirtualMachineState);

  int32_t offsetCount = high - low + 1;

  std::vector<int32_t> jumpTable(offsetCount);
  for (int i = 0; i < offsetCount; ++i)
  {
    jumpTable[i] = Read32BitOffset(pVirtualMachineState);
  }

  boost::intrusive_ptr< JavaInteger > pIndex = boost::dynamic_pointer_cast<JavaInteger>(pVirtualMachineState->PopOperand());

  intptr_t targetAddress = 0;
  if (pIndex->ToHostInt32() < low || pIndex->ToHostInt32() > high)
  {
    targetAddress = startingAddress + deflt;
  }
  else
  {
    int32_t jumpTableIndex = pIndex->ToHostInt32() - low;
    targetAddress = startingAddress + jumpTable[jumpTableIndex];
  }

  pVirtualMachineState->AdvanceProgramCounter(targetAddress - pVirtualMachineState->GetProgramCounter());
}

void BasicExecutionEngine::ExecuteOpCodeBranchIfNull(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int16_t offset = ReadOffset(pVirtualMachineState);

  boost::intrusive_ptr<IJavaVariableType> pReference = pVirtualMachineState->PopOperand();
  if (pReference->IsNull())
  {
    AdjustProgramCounterByOffset(pVirtualMachineState, offset);
  }
}

// TODO: Debug here
void BasicExecutionEngine::ExecuteOpCodeBranchIfReferencesAreEqual(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int16_t offset = ReadOffset(pVirtualMachineState);

  boost::intrusive_ptr<IJavaVariableType> pValue1 = pVirtualMachineState->PopOperand();
  if (!pValue1->IsReferenceType())
  {
    throw InvalidStateException(__FUNCTION__ " - Expected first operand to be an a reference type.");
  }

  boost::intrusive_ptr<IJavaVariableType> pValue2 = pVirtualMachineState->PopOperand();
  if (!pValue2->IsReferenceType())
  {
    throw InvalidStateException(__FUNCTION__ " - Expected second operand to be an a reference type.");
  }

  bool areValuesEqual = false;
  if (pValue2->GetVariableType() == pValue1->GetVariableType())
  {
    areValuesEqual = boost::dynamic_pointer_cast<ObjectReference>(pValue1)->GetIndex() == boost::dynamic_pointer_cast<ObjectReference>(pValue2)->GetIndex();
  }

  if (areValuesEqual)
  {
    AdjustProgramCounterByOffset(pVirtualMachineState, offset);
  }
}

void BasicExecutionEngine::ExecuteOpCodeBranchIfReferencesAreNotEqual(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int16_t offset = ReadOffset(pVirtualMachineState);

  boost::intrusive_ptr<IJavaVariableType> pValue1 = pVirtualMachineState->PopOperand();
  if (!pValue1->IsReferenceType())
  {
    throw InvalidStateException(__FUNCTION__ " - Expected first operand to be an a reference type.");
  }

  boost::intrusive_ptr<IJavaVariableType> pValue2 = pVirtualMachineState->PopOperand();
  if (!pValue2->IsReferenceType())
  {
    throw InvalidStateException(__FUNCTION__ " - Expected second operand to be an a reference type.");
  }

  bool areValuesEqual = false;
  if (pValue2->GetVariableType() == pValue1->GetVariableType())
  {
    areValuesEqual = boost::dynamic_pointer_cast<ObjectReference>(pValue1)->GetIndex() == boost::dynamic_pointer_cast<ObjectReference>(pValue2)->GetIndex();
    //areValuesEqual = (pValue2-> == *pValue1);
  }

  if (!areValuesEqual)
  {
    AdjustProgramCounterByOffset(pVirtualMachineState, offset);
  }
}

void BasicExecutionEngine::ExecuteOpCodeThrowReference(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  auto pOperand = pVirtualMachineState->PopOperand();
  if (!pOperand->IsReferenceType())
  {
    throw InvalidStateException(__FUNCTION__  " - Expected a reference type when throwing exception.");
  }

  if (pOperand->IsNull())
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaNullPointerExceptionException);
    return;
  }

  boost::intrusive_ptr<ObjectReference> pObject = boost::dynamic_pointer_cast<ObjectReference>(pOperand);
  if (nullptr == pObject)
  {
    throw NotImplementedException(__FUNCTION__ " - Arrays not yet implemented.");
  }

  if (!IsInstanceOf(pVirtualMachineState, new JavaString(c_ThrowableClassName), pObject->GetContainedObject()->GetClass()->GetName()))
  {
    throw InvalidStateException(__FUNCTION__ " - Expected class to implement Throwable.");
  }

  HelperClasses::ThrowJavaExceptionInternal(pVirtualMachineState.get(), pObject);
}

bool BasicExecutionEngine::CanExceptionBeHandledLocally(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, intptr_t programCounterBeforeLastInstruction)
{
  return nullptr != FindLocalExceptionTableEntry(pVirtualMachineState, programCounterBeforeLastInstruction);
}

std::shared_ptr<ExceptionTableEntry> BasicExecutionEngine::FindLocalExceptionTableEntry(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, intptr_t programCounterBeforeLastInstruction)
{
  std::shared_ptr< MethodInfo > pMethodInfo = pVirtualMachineState->GetCurrentMethodInfo();
  if (nullptr == pMethodInfo)
  {
    throw InvalidStateException(__FUNCTION__ " - Expected to get a method info backs.");
  }

  const ClassAttributeCode* pCodeAttributes = pMethodInfo->GetCodeInfo();
  if (nullptr == pCodeAttributes)
  {
    throw InvalidStateException(__FUNCTION__ " - Expected code attributes to be non-null.");
  }

  const ExceptionTable& exceptionTable = pCodeAttributes->GetExceptionTable();
  for (auto it : exceptionTable)
  {
    std::shared_ptr<ConstantPoolClassReference> pCatchType = it.GetCatchType();
    if (nullptr == pCatchType ||
      IsInstanceOf(pVirtualMachineState, pCatchType->GetClassName(), pVirtualMachineState->GetException()->GetContainedObject()->GetClass()->GetName()))
    {
      if (programCounterBeforeLastInstruction >= it.GetStartPosition() &&
        programCounterBeforeLastInstruction < it.GetEndPosition())
      {
        return std::make_shared<ExceptionTableEntry>(it);
      }
    }
  }

  return nullptr;
}

void BasicExecutionEngine::CatchException(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, intptr_t programCounterBeforeLastInstruction)
{
  std::shared_ptr<ExceptionTableEntry> pEntry = FindLocalExceptionTableEntry(pVirtualMachineState, programCounterBeforeLastInstruction);
  if (nullptr == pEntry)
  {
    throw InvalidStateException(__FUNCTION__ " - Expected exception table entry to be valid.");
  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Exception caught in %s (%s)", pVirtualMachineState->GetCurrentClassAndMethodName().ToUtf8String().c_str(), pVirtualMachineState->GetException()->GetContainedObject()->ToString().ToUtf8String().c_str());
  }
#endif // defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)

  int programCounter = pVirtualMachineState->GetProgramCounter();

#ifdef _DEBUG
  if (programCounter > pEntry->GetHandlerPosition())
  {
    JVMX_ASSERT(false);
  }
#endif // _DEBUG

  uint16_t adjustment = static_cast<uint16_t>(pEntry->GetHandlerPosition() - programCounter);
  JVMX_ASSERT(adjustment < 0xF000);

  // the operand stack of the current frame is cleared,
  RewindOperandStack(pVirtualMachineState, pEntry->GetStartPosition()); // Rewindin must happen first, because we use the current PC.

  // If an exception handler that matches objectref is found, it contains
  // the location of the code intended to handle this exception.The pc
  // register is reset to that location,
  pVirtualMachineState->AdvanceProgramCounter(adjustment);

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Exception caught in %s(%s), %s", pVirtualMachineState->GetCurrentClassAndMethodName().ToUtf8String().c_str(), pVirtualMachineState->GetCurrentMethodType().ToUtf8String().c_str(), pVirtualMachineState->GetException()->GetContainedObject()->ToString().ToUtf8String().c_str());
  }
#endif // defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)

  // objectref is pushed back onto the operand stack,  and execution continues
  pVirtualMachineState->PushOperand(pVirtualMachineState->GetException());
  pVirtualMachineState->ResetException();
}

void BasicExecutionEngine::RewindOperandStack(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, size_t exceptionStartRegion)
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Before rewinding operand stack:");
    pVirtualMachineState->LogOperandStack();
  }
#endif // defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)

  size_t numberOfFieldsToClear = pVirtualMachineState->CalculateNumberOfStackItemsToClear(exceptionStartRegion);

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Popping %lld objects from the operand stack.", (int64_t)numberOfFieldsToClear);
  }
#endif // defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)

  for (size_t i = 0; i < numberOfFieldsToClear; ++i)
  {
    pVirtualMachineState->PopOperand();
  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("After rewinding operand stack:");
    pVirtualMachineState->LogOperandStack();
  }
#endif // defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
}

void BasicExecutionEngine::ExecuteOpCodeJumpSubRoutine(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int16_t offset = ReadOffset(pVirtualMachineState);

  boost::intrusive_ptr<JavaReturnAddress> pAddress = new JavaReturnAddress(pVirtualMachineState->GetProgramCounter());
  pVirtualMachineState->PushOperand(pAddress);

  AdjustProgramCounterByOffset(pVirtualMachineState, offset);
}

void BasicExecutionEngine::ExecuteOpCodeReturnFromSubRoutine(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  //auto programCounter = pVirtualMachineState->GetProgramCounter();
  uint8_t localVariableIndex = ReadByteUnsigned(pVirtualMachineState); // This will move the PC, so we have to take the PC before this for our calculation.

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogLocalVariables();
    GetLogger()->LogDebug("Loading return address: %s from local variable %d", pVirtualMachineState->GetLocalVariable(localVariableIndex)->ToString().ToUtf8String().c_str(), static_cast<int>(localVariableIndex));
  }
#endif

  boost::intrusive_ptr<IJavaVariableType> pLocalVariable = pVirtualMachineState->GetLocalVariable(localVariableIndex);
  if (e_JavaVariableTypes::ReturnAddress != pLocalVariable->GetVariableType())
  {
    throw InvalidStateException(__FUNCTION__ " - Expected return address as local variable.");
  }

  boost::intrusive_ptr<JavaReturnAddress> pAddress = boost::dynamic_pointer_cast<JavaReturnAddress>(pLocalVariable);

  int16_t offset = static_cast<uint16_t>(pAddress->GetAddress() - pVirtualMachineState->GetProgramCounter());
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Destination Address = %d, calculated offset = %d", pAddress->GetAddress(), offset);
  }
#endif

  pVirtualMachineState->AdvanceProgramCounter(offset);

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("PC after adjustment = %lu", pVirtualMachineState->GetProgramCounter());
  }
#endif
}

void BasicExecutionEngine::ExecuteOpCodeReturnLong(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#ifdef _DEBUG
  const size_t debugSize = pVirtualMachineState->GetOperandStackSize();
#endif // _DEBUG

  std::shared_ptr<MethodInfo> pMethodInfo = pVirtualMachineState->GetCurrentMethodInfo();
  if (pMethodInfo->IsSynchronised())
  {
    pVirtualMachineState->PopMonitor()->Unlock(pMethodInfo->GetFullName().ToUtf8String().c_str());
  }

  if (pVirtualMachineState->HasExceptionOccurred())
  {
    return;
  }

  boost::intrusive_ptr<IJavaVariableType> pReference = pVirtualMachineState->PopOperand();
  if (pReference->GetVariableType() != e_JavaVariableTypes::Long)
  {
    throw InvalidStateException(__FUNCTION__ " - Value on the operand stack was not of type long.");
  }

  pVirtualMachineState->PopState();
  pVirtualMachineState->ReleaseLocalVariables();

  pVirtualMachineState->PushOperand(pReference);

#ifdef _DEBUG
  JVMX_ASSERT(pVirtualMachineState->GetOperandStackSize() == debugSize);
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeReturnDouble(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#ifdef _DEBUG
  const size_t debugSize = pVirtualMachineState->GetOperandStackSize();
#endif // _DEBUG

  std::shared_ptr<MethodInfo> pMethodInfo = pVirtualMachineState->GetCurrentMethodInfo();
  if (pMethodInfo->IsSynchronised())
  {
    pVirtualMachineState->PopMonitor()->Unlock(pMethodInfo->GetFullName().ToUtf8String().c_str());
  }

  if (pVirtualMachineState->HasExceptionOccurred())
  {
    return;
  }

  boost::intrusive_ptr<IJavaVariableType> pReference = pVirtualMachineState->PopOperand();
  if (pReference->GetVariableType() != e_JavaVariableTypes::Double)
  {
    throw InvalidStateException(__FUNCTION__ " - Value on the operand stack was not of type double.");
  }

  pVirtualMachineState->PopState();
  pVirtualMachineState->ReleaseLocalVariables();

  pVirtualMachineState->PushOperand(pReference);

#ifdef _DEBUG
  JVMX_ASSERT(pVirtualMachineState->GetOperandStackSize() == debugSize);
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeReturnFloat(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#ifdef _DEBUG
  const size_t debugSize = pVirtualMachineState->GetOperandStackSize();
#endif // _DEBUG

  std::shared_ptr<MethodInfo> pMethodInfo = pVirtualMachineState->GetCurrentMethodInfo();
  if (pMethodInfo->IsSynchronised())
  {
    pVirtualMachineState->PopMonitor()->Unlock(pMethodInfo->GetFullName().ToUtf8String().c_str());
  }

  if (pVirtualMachineState->HasExceptionOccurred())
  {
    return;
  }

  boost::intrusive_ptr<IJavaVariableType> pReference = pVirtualMachineState->PopOperand();
  if (pReference->GetVariableType() != e_JavaVariableTypes::Float)
  {
    throw InvalidStateException(__FUNCTION__ " - Value on the operand stack was not of type float.");
  }

  pVirtualMachineState->PopState();
  pVirtualMachineState->ReleaseLocalVariables();

  pVirtualMachineState->PushOperand(pReference);

#ifdef _DEBUG
  JVMX_ASSERT(pVirtualMachineState->GetOperandStackSize() == debugSize);
#endif // _DEBUG
}

void BasicExecutionEngine::HandleUnhandledException(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  JVMX_ASSERT(pVirtualMachineState->HasExceptionOccurred());

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Unhandled Exception");
  }
#endif // _DEBUG

  auto exception = pVirtualMachineState->GetException();
  auto threadInfo = pVirtualMachineState->ReturnCurrentThreadInfo();

  //"getUncaughtExceptionHandler"

  auto pField = threadInfo.m_pThreadObject->GetContainedObject()->GetFieldByName(JavaString::FromCString(u"group"));
  if (nullptr == pField)
  {
    GetLogger()->LogError("Error trying to handle uncaught Java exception. Could not find field 'group' on thread.");
    return;
  }

  if (e_JavaVariableTypes::Object != pField->GetVariableType())
  {
    GetLogger()->LogError("Error trying to handle uncaught Java exception. Field 'group' was expected to be an object.");
    return;
  }

  boost::intrusive_ptr<ObjectReference> pFieldObject = new ObjectReference(*dynamic_cast<const ObjectReference*>(pField.get()));
  if (nullptr == pFieldObject)
  {
    GetLogger()->LogError("Error trying to handle uncaught Java exception. Could not cast field value to object.");
    return;
  }

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  auto threadGroupClassName = pFieldObject->GetContainedObject()->GetClass()->GetName();

  pVirtualMachineState->PushOperand(pField);
  pVirtualMachineState->PushOperand(threadInfo.m_pThreadObject);
  pVirtualMachineState->PushOperand(pVirtualMachineState->GetException());

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Operand stack before resetting exception.");
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  pVirtualMachineState->ResetException();

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  pVirtualMachineState->Execute(*threadGroupClassName.get(), c_UncaughtExceptionMethodName, c_UncaughtExceptionMethodType);

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Unhandled Exception was handled by threadgroup.");
  }
#endif // _DEBUG

  // We have handeled it, and we needed to clear it so the code above can run, but now we are setting it again, 
  // so we can detect it up-stream.
  pVirtualMachineState->SetExceptionThrown(exception);

  // TODO: We may have to do some more stuff here to kill the thread!
  // I think we are safe, because in theory only the main thread will ever get here and all that is left is cleanup.
}
