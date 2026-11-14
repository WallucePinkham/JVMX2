#include <cstdlib>
#include <cfloat>

#include <cstring>

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
#include "DebugBreak.h"

#if 0
#define __EXTRA_LOGGING
#endif

extern const JavaString c_ClassInitialisationMethodName;
extern const JavaString c_InstanceInitialisationMethodName;

const JavaString c_MethodTypeClassName = JavaString::FromCString(JVMX_T("java/lang/invoke/MethodType"));
extern const JavaString c_ThrowableClassName = JavaString::FromCString(JVMX_T("java/lang/Throwable"));

extern const JavaString c_UncaughtExceptionMethodName = JavaString::FromCString(JVMX_T("uncaughtException"));
extern const JavaString c_UncaughtExceptionMethodType = JavaString::FromCString(JVMX_T("(Ljava/lang/Thread;Ljava/lang/Throwable;)V"));

class StackLevelIncrementer
{
public:
  explicit StackLevelIncrementer(std::atomic_int64_t& level)
    : m_Level(level)
  {
    ++m_Level;
  }

  ~StackLevelIncrementer()
  {
    --m_Level;
    JVMX_ASSERT(m_Level >= 0);
  }

private:
  std::atomic_int64_t& m_Level;
};

BasicExecutionEngine::BasicExecutionEngine()
  : m_Halted(false)
{
#ifdef _DEBUG
  m_InstructionsExecuted = 0;
#endif // _DEBUG
}

BasicExecutionEngine::~BasicExecutionEngine()
{
}

void BasicExecutionEngine::Run(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  StackLevelIncrementer incrementer(pVirtualMachineState->GetStackLevel());
  intptr_t savedProgramCounter = 0; // Save the program counter before the next instruction is executed, for exception handling.

  std::shared_ptr<IGarbageCollector> pGarbageCollector = GlobalCatalog::GetInstance().Get("GarbageCollector");
  pVirtualMachineState->PushAndZeroCallStackDepth();

  while (!m_Halted && pVirtualMachineState->GetProgramCounter() < pVirtualMachineState->GetCodeSegmentLength())
  {
    if (pVirtualMachineState->IsPausing())
    {
      pVirtualMachineState->ConfirmPaused();
    }

    if (pVirtualMachineState->IsPaused())
    {
      std::this_thread::yield();
      continue;
    }

    TryDoGarbageCollection(pVirtualMachineState, pGarbageCollector);

    if (!pVirtualMachineState->HasExceptionOccurred())
    {
      if (e_ImmediateReturnRequired::Yes == ProcessNextOpcode(pVirtualMachineState, GetLogger()))
      {
        return;
      }
    }
    // Don't put an else here, because ProcessNextOpCode will set the flag to indicate an Exception has occurred.

    if (pVirtualMachineState->HasExceptionOccurred())
    {
      savedProgramCounter = pVirtualMachineState->GetProgramCounter() - 1;

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
      GetLogger()->LogDebug("Exception occurred in %s (%s) : %lld", pVirtualMachineState->GetCurrentClassAndMethodName().ToUtf8String().c_str(), pVirtualMachineState->GetException()->GetContainedObject()->ToString().ToUtf8String().c_str(), savedProgramCounter);
#endif // _DEBUG

      if (!CanExceptionBeHandledLocally(pVirtualMachineState, savedProgramCounter))
      {
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
        GetLogger()->LogDebug("Exception cannot be handled in this function. Rewinding the stack, and executing a \"void\" return.");
#endif // _DEBUG

        RewindOperandStack(pVirtualMachineState, 0);
        ExecuteOpCodeReturnVoid(pVirtualMachineState);

        if (0 == pVirtualMachineState->GetCallStackDepth())
        {
          pVirtualMachineState->PopCallStackDepth();
          if (pVirtualMachineState->GetStackLevel() == 1)
          {
            HandleUnhandledException(pVirtualMachineState);
          }
          return;
        }
        else
        {
          pVirtualMachineState->DecrementCallStackDepth();
        }
      }
      else
      {
        CatchException(pVirtualMachineState, savedProgramCounter);
      }
    }
  }
}

void BasicExecutionEngine::TryDoGarbageCollection(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, const std::shared_ptr<IGarbageCollector>& pGarbageCollector)
{
#ifdef _DEBUG
  if (m_InstructionsExecuted > 0 && 0 == (m_InstructionsExecuted % 500000))
  {
    GetLogger()->LogDebug("%lld Instructions executed. MustCollect( %s ), StackLevel( %lld )", m_InstructionsExecuted, pGarbageCollector->MustCollect() ? "true" : "false", pVirtualMachineState->GetStackLevel().load());
  }
#endif // _DEBUG

#ifdef _DEBUG
  if ((m_InstructionsExecuted > 0 && 0 == (m_InstructionsExecuted % 500000)) || pGarbageCollector->MustCollect())
#else
  if (pGarbageCollector->MustCollect())
#endif // _DEBUG

  {

#ifdef JVMX_LOG_VERBOSE
    GetLogger()->LogDebug("MustCollect( %s )", pGarbageCollector->MustCollect() ? "true" : "false");

    if (pVirtualMachineState->HasUserCodeStarted())
    {
      pVirtualMachineState->LogCallStack();
      pVirtualMachineState->LogLocalVariables();
      pVirtualMachineState->LogOperandStack();
    }
#endif // _DEBUG

    pGarbageCollector->Collect();

#ifdef JVMX_LOG_VERBOSE
    if (pVirtualMachineState->HasUserCodeStarted())
    {
      pVirtualMachineState->LogCallStack();
      pVirtualMachineState->LogLocalVariables();
      pVirtualMachineState->LogOperandStack();
    }
#endif // _DEBUG

  }
}

e_ImmediateReturnRequired BasicExecutionEngine::ProcessNextOpcode(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, std::shared_ptr<ILogger> pLogger)
{
  uint16_t opCode = GetNextInstruction(pVirtualMachineState);

#ifdef JVMX_LOG_VERBOSE
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pLogger->LogDebug("%s %Id - Read opcode: %hu (%s)\n", pVirtualMachineState->GetCurrentClassAndMethodName().ToUtf8String().c_str(), pVirtualMachineState->GetProgramCounter() - 1, opCode, TranslateOpCode(opCode));
  }
#endif // _DEBUG

#if defined( _DEBUG ) && defined (__EXTRA_LOGGING)
  //BreakDebug( pVirtualMachineState->GetCurrentClassAndMethodName().ToUtf16String().c_str(), u"java/awt/color/ICC_Profile::createLinearRGBProfile" );
  static bool extraDebugLogging = false;
  static std::thread::id debugThreadId = std::thread::id();

  if (pVirtualMachineState->GetCurrentClassAndMethodName().Contains(u"findClass"))
  {
    extraDebugLogging = true;
    debugThreadId = std::this_thread::get_id();
  }

  if (extraDebugLogging &&
    std::this_thread::get_id() == debugThreadId && (
      //pVirtualMachineState->GetCurrentClassAndMethodName().EndsWith( u"createProfile" ) ||
      //pVirtualMachineState->GetCurrentClassAndMethodName().EndsWith( u"setData" ) ||
      //pVirtualMachineState->GetCurrentClassAndMethodName().EndsWith( u"QtComponentPeer::updateBackBuffer" ) ||
      //pVirtualMachineState->GetCurrentClassAndMethodName().EndsWith( u"QtImage::createImage" ) ||
      /*( pVirtualMachineState->GetCurrentClassAndMethodName().EndsWith( u"Reference::get" )  && pVirtualMachineState->GetProgramCounter() >= 12 ) ||
      ( pVirtualMachineState->GetCurrentClassAndMethodName().EndsWith( u"AWTKeyStroke::<clinit>" ) && pVirtualMachineState->GetProgramCounter() >= 32 ) || /**/
      pVirtualMachineState->GetCurrentClassAndMethodName().EndsWith(u"URLClassLoader::findClass")
      //&& pVirtualMachineState->GetCurrentMethodType().EndsWith(u"m;)V")
      ))
  {
    //if ( pVirtualMachineState->GetProgramCounter() >= 17 )
    {
      pVirtualMachineState->SetUserCodeStarted();
      pLogger->LogDebug("@@Inside suspect method:");
      pVirtualMachineState->LogLocalVariables();
      pVirtualMachineState->LogOperandStack();
      pVirtualMachineState->LogCallStack();
    }
  }

#if 0
  if (extraDebugLogging && std::this_thread::get_id() == debugThreadId)
  {
    pLogger->LogDebug("%s(%s) %Id - Read opcode: %hu (%s)\n", pVirtualMachineState->GetCurrentClassAndMethodName().ToUtf8String().c_str(), pVirtualMachineState->GetCurrentMethodType().ToUtf8String().c_str(), pVirtualMachineState->GetProgramCounter() - 1, opCode, TranslateOpCode(opCode));
    int i = 0;
  }
#endif 
#endif // _DEBUG

#ifdef _DEBUG
  ++m_InstructionsExecuted;
#endif // _DEBUG

  switch ((e_JavaOpCodes)opCode)
  {
  case e_JavaOpCodes::PushNull:
    ExecuteOpCodePushNull(pVirtualMachineState);
    break;

  case e_JavaOpCodes::PushInt_Minus1:
    ExecuteOpCodePushInt(pVirtualMachineState, -1);
    break;

  case e_JavaOpCodes::PushInt_0:
    ExecuteOpCodePushInt(pVirtualMachineState, 0);
    break;

  case e_JavaOpCodes::PushInt_1:
    ExecuteOpCodePushInt(pVirtualMachineState, 1);
    break;

  case e_JavaOpCodes::PushInt_2:
    ExecuteOpCodePushInt(pVirtualMachineState, 2);
    break;

  case e_JavaOpCodes::PushInt_3:
    ExecuteOpCodePushInt(pVirtualMachineState, 3);
    break;

  case e_JavaOpCodes::PushInt_4:
    ExecuteOpCodePushInt(pVirtualMachineState, 4);
    break;

  case e_JavaOpCodes::PushInt_5:
    ExecuteOpCodePushInt(pVirtualMachineState, 5);
    break;

  case e_JavaOpCodes::PushLong_0:
    ExecuteOpCodePushLong(pVirtualMachineState, 0);
    break;

  case e_JavaOpCodes::PushLong_1:
    ExecuteOpCodePushLong(pVirtualMachineState, 1);
    break;

  case e_JavaOpCodes::PushInt_ImmediateByte:
    ExecuteOpCodePushIntImmediateByte(pVirtualMachineState);
    break;

  case e_JavaOpCodes::PushFromConstantPoolWide:
    ExecuteOpCodePushFromConstantPoolWide(pVirtualMachineState);
    break;

  case e_JavaOpCodes::PushFloatConstant_0:
    ExecuteOpCodePushFloatConstant(pVirtualMachineState, 0.0f);
    break;

  case e_JavaOpCodes::PushFloatConstant_1:
    ExecuteOpCodePushFloatConstant(pVirtualMachineState, 1.0f);
    break;

  case e_JavaOpCodes::PushFloatConstant_2:
    ExecuteOpCodePushFloatConstant(pVirtualMachineState, 2.0f);
    break;

  case e_JavaOpCodes::PushShortConstant:
    ExecuteOpCodePushShortConstant(pVirtualMachineState);
    break;

  case e_JavaOpCodes::LoadReferenceFromArray:
    ExecuteOpCodeLoadReferenceFromArray(pVirtualMachineState);
    break;

  case e_JavaOpCodes::LoadCharacterFromArray:
    ExecuteOpCodeLoadCharacterFromArray(pVirtualMachineState);
    break;

  case e_JavaOpCodes::LoadShortFromArray:
    ExecuteOpCodeLoadShortFromArray(pVirtualMachineState);
    break;

  case e_JavaOpCodes::StoreLongInLocal:
    ExecuteOpCodeStoreLongInLocalWithIndex(pVirtualMachineState);
    break;

  case e_JavaOpCodes::StoreLongInLocal_0:
    ExecuteOpCodeStoreLongInLocal(pVirtualMachineState, 0);
    break;

  case e_JavaOpCodes::StoreLongInLocal_1:
    ExecuteOpCodeStoreLongInLocal(pVirtualMachineState, 1);
    break;

  case e_JavaOpCodes::StoreLongInLocal_2:
    ExecuteOpCodeStoreLongInLocal(pVirtualMachineState, 2);
    break;

  case e_JavaOpCodes::StoreLongInLocal_3:
    ExecuteOpCodeStoreLongInLocal(pVirtualMachineState, 3);
    break;

  case e_JavaOpCodes::StoreReferenceInLocal:
    ExecuteOpCodeStoreReferenceInLocalWithIndex(pVirtualMachineState);
    break;

  case e_JavaOpCodes::StoreReferenceInLocal_0:
    ExecuteOpCodeStoreReferenceInLocal(pVirtualMachineState, 0);
    break;

  case e_JavaOpCodes::StoreReferenceInLocal_1:
    ExecuteOpCodeStoreReferenceInLocal(pVirtualMachineState, 1);
    break;

  case e_JavaOpCodes::StoreReferenceInLocal_2:
    ExecuteOpCodeStoreReferenceInLocal(pVirtualMachineState, 2);
    break;

  case e_JavaOpCodes::StoreReferenceInLocal_3:
    ExecuteOpCodeStoreReferenceInLocal(pVirtualMachineState, 3);
    break;

  case e_JavaOpCodes::StoreIntegerInLocal:
    ExecuteOpCodeStoreIntegerInLocalWithIndex(pVirtualMachineState);
    break;

  case e_JavaOpCodes::StoreIntegerInLocal_0:
    ExecuteOpCodeStoreIntegerInLocal(pVirtualMachineState, 0);
    break;

  case e_JavaOpCodes::StoreIntegerInLocal_1:
    ExecuteOpCodeStoreIntegerInLocal(pVirtualMachineState, 1);
    break;

  case e_JavaOpCodes::StoreIntegerInLocal_2:
    ExecuteOpCodeStoreIntegerInLocal(pVirtualMachineState, 2);
    break;

  case e_JavaOpCodes::StoreIntegerInLocal_3:
    ExecuteOpCodeStoreIntegerInLocal(pVirtualMachineState, 3);
    break;

  case e_JavaOpCodes::LoadIntegerFromLocal:
    ExecuteOpCodeLoadIntegerFromLocalWithIndex(pVirtualMachineState);
    break;

  case e_JavaOpCodes::LoadLongFromLocal:
    ExecuteOpCodeLoadLongFromLocalWithIndex(pVirtualMachineState);
    break;

  case e_JavaOpCodes::LoadIntegerFromLocal_0:
    ExecuteOpCodeLoadIntegerFromLocal(pVirtualMachineState, 0);
    break;

  case e_JavaOpCodes::LoadIntegerFromLocal_1:
    ExecuteOpCodeLoadIntegerFromLocal(pVirtualMachineState, 1);
    break;

  case e_JavaOpCodes::LoadIntegerFromLocal_2:
    ExecuteOpCodeLoadIntegerFromLocal(pVirtualMachineState, 2);
    break;

  case e_JavaOpCodes::LoadIntegerFromLocal_3:
    ExecuteOpCodeLoadIntegerFromLocal(pVirtualMachineState, 3);
    break;

  case e_JavaOpCodes::LoadLongFromLocal_0:
    ExecuteOpCodeLoadLongFromLocal(pVirtualMachineState, 0);
    break;

  case e_JavaOpCodes::LoadLongFromLocal_1:
    ExecuteOpCodeLoadLongFromLocal(pVirtualMachineState, 1);
    break;

  case e_JavaOpCodes::LoadLongFromLocal_2:
    ExecuteOpCodeLoadLongFromLocal(pVirtualMachineState, 2);
    break;

  case e_JavaOpCodes::LoadLongFromLocal_3:
    ExecuteOpCodeLoadLongFromLocal(pVirtualMachineState, 3);
    break;

  case e_JavaOpCodes::LoadFloatFromLocal:
    ExecuteOpCodeLoadFloatFromLocalWithIndex(pVirtualMachineState);
    break;

  case e_JavaOpCodes::LoadFloatFromLocal_0:
    ExecuteOpCodeLoadFloatFromLocal(pVirtualMachineState, 0);
    break;

  case e_JavaOpCodes::LoadFloatFromLocal_1:
    ExecuteOpCodeLoadFloatFromLocal(pVirtualMachineState, 1);
    break;

  case e_JavaOpCodes::LoadFloatFromLocal_2:
    ExecuteOpCodeLoadFloatFromLocal(pVirtualMachineState, 2);
    break;

  case e_JavaOpCodes::LoadFloatFromLocal_3:
    ExecuteOpCodeLoadFloatFromLocal(pVirtualMachineState, 3);
    break;

  case e_JavaOpCodes::GetStatic:
    ExecuteOpCodeGetStatic(pVirtualMachineState);
    break;

  case e_JavaOpCodes::PutStatic:
    ExecuteOpCodePutStatic(pVirtualMachineState);
    break;

  case e_JavaOpCodes::GetField:
    ExecuteOpCodeGetField(pVirtualMachineState);
    break;

  case e_JavaOpCodes::PutField:
    ExecuteOpCodePutField(pVirtualMachineState);
    break;

  case e_JavaOpCodes::LoadReferenceFromLocal:
    ExecuteOpCodeLoadReferenceFromLocalWithSpecifiedIndex(pVirtualMachineState);
    break;

  case e_JavaOpCodes::LoadReferenceFromLocal_0:
    ExecuteOpCodeLoadReferenceFromLocalIndex(pVirtualMachineState, 0);
    break;

  case e_JavaOpCodes::LoadReferenceFromLocal_1:
    ExecuteOpCodeLoadReferenceFromLocalIndex(pVirtualMachineState, 1);
    break;

  case e_JavaOpCodes::LoadReferenceFromLocal_2:
    ExecuteOpCodeLoadReferenceFromLocalIndex(pVirtualMachineState, 2);
    break;

  case e_JavaOpCodes::LoadReferenceFromLocal_3:
    ExecuteOpCodeLoadReferenceFromLocalIndex(pVirtualMachineState, 3);
    break;

  case e_JavaOpCodes::InvokeVirtual:
    if (e_IncreaseCallStackDepth::Yes == ExecuteOpCodeInvokeVirtual(pVirtualMachineState))
    {
      pVirtualMachineState->IncrementCallStackDepth();
    }
    break;

  case e_JavaOpCodes::InvokeSpecial:
    if (e_IncreaseCallStackDepth::Yes == ExecuteOpCodeInvokeSpecial(pVirtualMachineState))
    {
      pVirtualMachineState->IncrementCallStackDepth();
    }
    break;

  case e_JavaOpCodes::InvokeStatic:
    if (e_IncreaseCallStackDepth::Yes == ExecuteOpCodeInvokeStatic(pVirtualMachineState))
    {
      pVirtualMachineState->IncrementCallStackDepth();
    }
    break;

  case e_JavaOpCodes::InvokeInterface:
    if (e_IncreaseCallStackDepth::Yes == ExecuteOpCodeInvokeInterfaceMethod(pVirtualMachineState))
    {
      pVirtualMachineState->IncrementCallStackDepth();
    }
    break;

  case e_JavaOpCodes::StoreIntoIntArray:
    ExecuteOpCodeStoreIntoIntArray(pVirtualMachineState);
    break;

  case e_JavaOpCodes::StoreIntoLongArray:
    ExecuteOpCodeStoreIntoLongArray(pVirtualMachineState);
    break;

  case e_JavaOpCodes::DuplicateTopOperand:
    ExecuteOpCodeDuplicateTopOperand(pVirtualMachineState);
    break;

  case e_JavaOpCodes::DuplicateTopOperandx1:
    ExecuteOpCodeDuplicateTopOperandx1(pVirtualMachineState);
    break;

  case e_JavaOpCodes::PopOperandStack:
    ExecuteOpCodePopOperandStack(pVirtualMachineState);
    break;

  case e_JavaOpCodes::StoreIntoCharArray:
    ExecuteOpCodeStoreIntoCharArray(pVirtualMachineState);
    break;

  case e_JavaOpCodes::StoreIntoShortArray:
    ExecuteOpCodeStoreIntoShortArray(pVirtualMachineState);
    break;

  case e_JavaOpCodes::StoreIntoReferenceArray:
    ExecuteOpCodeStoreIntoReferenceArray(pVirtualMachineState);
    break;

  case e_JavaOpCodes::IntegerAdd:
    ExecuteOpCodeIntegerAdd(pVirtualMachineState);
    break;

  case e_JavaOpCodes::IntegerSubtract:
    ExecuteOpCodeIntegerSubtract(pVirtualMachineState);
    break;

  case e_JavaOpCodes::LongSubtract:
    ExecuteOpCodeLongSubtract(pVirtualMachineState);
    break;

  case e_JavaOpCodes::IntegerMultiply:
    ExecuteOpCodeIntegerMultiply(pVirtualMachineState);
    break;

  case e_JavaOpCodes::FloatMultiply:
    ExecuteOpCodeFloatMultiply(pVirtualMachineState);
    break;

  case e_JavaOpCodes::IntegerDivide:
    ExecuteOpCodeIntegerDivide(pVirtualMachineState);
    break;

  case e_JavaOpCodes::LongDivide:
    ExecuteOpCodeLongDivide(pVirtualMachineState);
    break;

  case e_JavaOpCodes::IntegerRemainder:
    ExecuteOpCodeIntegerRemainder(pVirtualMachineState);
    break;

  case e_JavaOpCodes::LongRemainder:
    ExecuteOpCodeLongRemainder(pVirtualMachineState);
    break;

  case e_JavaOpCodes::FloatRemainder:
    ExecuteOpCodeFloatRemainder(pVirtualMachineState);
    break;

  case e_JavaOpCodes::DoubleRemainder:
    ExecuteOpCodeDoubleRemainder(pVirtualMachineState);
    break;

  case e_JavaOpCodes::NegateInteger:
    ExecuteOpCodeNegateInteger(pVirtualMachineState);
    break;

  case e_JavaOpCodes::ShiftIntegerLeft:
    ExecuteOpCodeShiftIntegerLeft(pVirtualMachineState);
    break;

  case e_JavaOpCodes::ShiftIntegerRightArithmetic:
    ExecuteOpCodeShiftIntegerRightArithmetic(pVirtualMachineState);
    break;

  case e_JavaOpCodes::IncrementLocalVariable:
    ExecuteOpCodeIncrementLocalVariableWithIndex(pVirtualMachineState);
    break;

  case e_JavaOpCodes::ConvertIntegerToFloat:
    ExecuteOpCodeConvertIntegerToFloat(pVirtualMachineState);
    break;

  case e_JavaOpCodes::ConvertIntegerToDouble:
    ExecuteOpCodeConvertIntegerToDouble(pVirtualMachineState);
    break;

  case e_JavaOpCodes::ConvertFloatToInteger:
    ExecuteOpCodeConvertFloatToInteger(pVirtualMachineState);
    break;

  case e_JavaOpCodes::ConvertIntegerToChar:
    ExecuteOpCodeConvertIntegerToChar(pVirtualMachineState);
    break;

  case e_JavaOpCodes::FloatingPointComparisonL:
    ExecuteOpCodeFloatComparisonL(pVirtualMachineState);
    break;

  case e_JavaOpCodes::FloatingPointComparisonG:
    ExecuteOpCodeFloatComparisonG(pVirtualMachineState);
    break;

  case e_JavaOpCodes::ReturnVoid:
  {
    ExecuteOpCodeReturnVoid(pVirtualMachineState);

    if (0 == pVirtualMachineState->GetCallStackDepth())
    {
      pVirtualMachineState->PopCallStackDepth();
      return e_ImmediateReturnRequired::Yes;
    }
    else
    {
      pVirtualMachineState->DecrementCallStackDepth();
    }
  }
  break;

  case e_JavaOpCodes::ReturnReference:
  {
    ExecuteOpCodeReturnReference(pVirtualMachineState);

    if (0 == pVirtualMachineState->GetCallStackDepth())
    {
      pVirtualMachineState->PopCallStackDepth();
      return e_ImmediateReturnRequired::Yes;
    }
    else
    {
      pVirtualMachineState->DecrementCallStackDepth();
    }
  }
  break;

  case e_JavaOpCodes::New:
    ExecuteOpCodeNew(pVirtualMachineState);
    break;

  case e_JavaOpCodes::NewArray:
    ExecuteOpCodeNewArray(pVirtualMachineState);
    break;

  case e_JavaOpCodes::NewArrayOfReference:
    ExecuteOpCodeNewArrayOfReference(pVirtualMachineState);
    break;

  case e_JavaOpCodes::ArrayLength:
    ExecuteOpCodeArrayLength(pVirtualMachineState);
    break;

  case e_JavaOpCodes::IsInstanceOf:
    ExecuteOpCodeIsInstanceOf(pVirtualMachineState);
    break;

  case e_JavaOpCodes::MonitorEnter:
    ExecuteOpCodeMonitorEnter(pVirtualMachineState);
    break;

  case e_JavaOpCodes::MonitorExit:
    ExecuteOpCodeMonitorExit(pVirtualMachineState);
    break;

  case e_JavaOpCodes::Wide:
    ExecuteOpCodeWide(pVirtualMachineState);
    break;

  case e_JavaOpCodes::CheckCast:
    ExecuteOpCodeCheckCast(pVirtualMachineState);
    break;

  case e_JavaOpCodes::NewMultiDimentionalArray:
    ExecuteOpCodeNewMultiDimentionalArray(pVirtualMachineState);
    break;

  case e_JavaOpCodes::LoadReferenceFromConstantPool:
    ExecuteOpCodeLoadReferenceFromConstantPool(pVirtualMachineState);
    break;

  case e_JavaOpCodes::BranchIfEquals:
    ExecutedOpBranchIfEquals(pVirtualMachineState);
    break;

  case e_JavaOpCodes::BranchIfNotEquals:
    ExecutedOpCodeBranchIfNotEquals(pVirtualMachineState);
    break;

  case e_JavaOpCodes::BranchIfLessThan:
    ExecutedOpCodeBranchIfLessThan(pVirtualMachineState);
    break;

  case e_JavaOpCodes::BranchIfGreaterThanEquals:
    ExecutedOpCodeBranchIfGreaterThanOrEqual(pVirtualMachineState);
    break;

  case e_JavaOpCodes::BranchIfGreaterThan:
    ExecutedOpCodeBranchIfGreaterThan(pVirtualMachineState);
    break;

  case e_JavaOpCodes::BranchIfLessThanEquals:
    ExecutedOpCodeBranchIfLessThanOrEqual(pVirtualMachineState);
    break;

  case e_JavaOpCodes::BranchIfIntegerEquals:
    ExecutedOpBranchIfIntegerEquals(pVirtualMachineState);
    break;

  case e_JavaOpCodes::BranchIfIntegerNotEquals:
    ExecutedOpBranchIfIntegerNotEquals(pVirtualMachineState);
    break;

  case e_JavaOpCodes::BranchIfIntegerLessThan:
    ExecutedOpBranchIfIntegerLessThan(pVirtualMachineState);
    break;

  case e_JavaOpCodes::BranchIfIntegerGreaterThanEquals:
    ExecutedOpBranchIfIntegerGreatherThanOrEqual(pVirtualMachineState);
    break;

  case e_JavaOpCodes::BranchIfIntegerGreaterThan:
    ExecutedOpBranchIfIntegerGreatherThan(pVirtualMachineState);
    break;

  case e_JavaOpCodes::BranchIfIntegerLessThanEquals:
    ExecutedOpBranchIfIntegerLessThanOrEqual(pVirtualMachineState);
    break;

  case e_JavaOpCodes::BranchIfReferencesAreEqual:
    ExecuteOpCodeBranchIfReferencesAreEqual(pVirtualMachineState);
    break;

  case e_JavaOpCodes::BranchIfReferencesAreNotEqual:
    ExecuteOpCodeBranchIfReferencesAreNotEqual(pVirtualMachineState);
    break;

  case e_JavaOpCodes::Goto:
    ExecuteOpCodeGoto(pVirtualMachineState);
    break;

  case e_JavaOpCodes::TableSwitch:
    ExecuteOpCodeTableSwitch(pVirtualMachineState);
    break;

  case e_JavaOpCodes::ReturnInteger:
    ExecuteOpCodeReturnInteger(pVirtualMachineState);

    if (0 == pVirtualMachineState->GetCallStackDepth())
    {
      pVirtualMachineState->PopCallStackDepth();
      return e_ImmediateReturnRequired::Yes;
    }
    else
    {
      pVirtualMachineState->DecrementCallStackDepth();
    }
    break;

  case e_JavaOpCodes::BranchIfNotNull:
    ExecuteOpCodeBranchIfNotNull(pVirtualMachineState);
    break;

  case e_JavaOpCodes::BranchIfNull:
    ExecuteOpCodeBranchIfNull(pVirtualMachineState);
    break;

  case e_JavaOpCodes::ConvertIntegerToByte:
    ExecuteOpCodeConvertIntegerToByte(pVirtualMachineState);
    break;

  case e_JavaOpCodes::StoreIntoByteArray:
    ExecuteOpCodeStoreIntoByteArray(pVirtualMachineState);
    break;

  case e_JavaOpCodes::LoadByteOrBooleanFromArray:
    ExecuteOpCodeLoadByteOrBooleanFromArray(pVirtualMachineState);
    break;

  case e_JavaOpCodes::IntegerXOR:
    ExecuteOpCodeXORInteger(pVirtualMachineState);
    break;

  case e_JavaOpCodes::ThrowReference:
    ExecuteOpCodeThrowReference(pVirtualMachineState);
    break;

  case e_JavaOpCodes::LongComparison:
    ExecuteOpCodeLongComparison(pVirtualMachineState);
    break;

  case e_JavaOpCodes::PushDoubleOrLongFromConstantPoolWide:
    ExecuteOpCodePushDoubleOrLongFromConstantPoolWide(pVirtualMachineState);
    break;

  case e_JavaOpCodes::JumpSubRoutine:
    ExecuteOpCodeJumpSubRoutine(pVirtualMachineState);
    break;

  case e_JavaOpCodes::ReturnFromSubRoutine:
    ExecuteOpCodeReturnFromSubRoutine(pVirtualMachineState);
    break;

  case e_JavaOpCodes::ReturnLong:
    ExecuteOpCodeReturnLong(pVirtualMachineState);

    if (0 == pVirtualMachineState->GetCallStackDepth())
    {
      pVirtualMachineState->PopCallStackDepth();
      return e_ImmediateReturnRequired::Yes;
    }
    else
    {
      pVirtualMachineState->DecrementCallStackDepth();
    }
    break;

  case e_JavaOpCodes::LongXOR:
    ExecuteOpCodeXORLong(pVirtualMachineState);
    break;

  case e_JavaOpCodes::LongAND:
    ExecuteOpCodeANDLong(pVirtualMachineState);
    break;

  case e_JavaOpCodes::LongMultiply:
    ExecuteOpCodeLongMultiply(pVirtualMachineState);
    break;

  case e_JavaOpCodes::LongAdd:
    ExecuteOpCodeLongAdd(pVirtualMachineState);
    break;

  case e_JavaOpCodes::ShiftLongRight:
    ExecuteOpCodeShiftLongRight(pVirtualMachineState);
    break;

  case e_JavaOpCodes::ShiftLongLeft:
    ExecuteOpCodeShiftLongLeft(pVirtualMachineState);
    break;

  case e_JavaOpCodes::ConvertLongToInteger:
    ExecuteOpCodeConvertLongToInteger(pVirtualMachineState);
    break;

  case e_JavaOpCodes::ConvertIntegerToLong:
    ExecuteOpCodeConvertIntegerToLong(pVirtualMachineState);
    break;

  case e_JavaOpCodes::ConvertLongToDouble:
    ExecuteOpCodeConvertLongToDouble(pVirtualMachineState);
    break;

  case e_JavaOpCodes::DoubleDivide:
    ExecuteOpCodeDoubleDivide(pVirtualMachineState);
    break;

  case e_JavaOpCodes::ReturnDouble:
    ExecuteOpCodeReturnDouble(pVirtualMachineState);

    if (0 == pVirtualMachineState->GetCallStackDepth())
    {
      pVirtualMachineState->PopCallStackDepth();
      return e_ImmediateReturnRequired::Yes;
    }
    else
    {
      pVirtualMachineState->DecrementCallStackDepth();
    }
    break;

  case e_JavaOpCodes::ReturnFloat:
    ExecuteOpCodeReturnFloat(pVirtualMachineState);

    if (0 == pVirtualMachineState->GetCallStackDepth())
    {
      pVirtualMachineState->PopCallStackDepth();
      return e_ImmediateReturnRequired::Yes;
    }
    else
    {
      pVirtualMachineState->DecrementCallStackDepth();
    }
    break;

  case e_JavaOpCodes::StoreDoubleInLocal:
    ExecuteOpCodeStoreDoubleInLocalWithIndex(pVirtualMachineState);
    break;

  case e_JavaOpCodes::StoreDoubleInLocal_0:
    ExecuteOpCodeStoreDoubleInLocal(pVirtualMachineState, 0);
    break;

  case e_JavaOpCodes::StoreDoubleInLocal_1:
    ExecuteOpCodeStoreDoubleInLocal(pVirtualMachineState, 1);
    break;

  case e_JavaOpCodes::StoreDoubleInLocal_2:
    ExecuteOpCodeStoreDoubleInLocal(pVirtualMachineState, 2);
    break;

  case e_JavaOpCodes::StoreDoubleInLocal_3:
    ExecuteOpCodeStoreDoubleInLocal(pVirtualMachineState, 3);
    break;

  case e_JavaOpCodes::LoadDoubleFromLocal:
    ExecuteOpCodeLoadDoubleFromLocalWithIndex(pVirtualMachineState);
    break;

  case e_JavaOpCodes::LoadDoubleFromLocal_0:
    ExecuteOpCodeLoadDoubleFromLocal(pVirtualMachineState, 0);

    break;

  case e_JavaOpCodes::LoadDoubleFromLocal_1:
    ExecuteOpCodeLoadDoubleFromLocal(pVirtualMachineState, 1);
    break;

  case e_JavaOpCodes::LoadDoubleFromLocal_2:
    ExecuteOpCodeLoadDoubleFromLocal(pVirtualMachineState, 2);
    break;

  case e_JavaOpCodes::LoadDoubleFromLocal_3:
    ExecuteOpCodeLoadDoubleFromLocal(pVirtualMachineState, 3);
    break;

  case e_JavaOpCodes::NoOperation:
    break;

  case e_JavaOpCodes::NegateDouble:
    ExecuteOpCodeNegateDouble(pVirtualMachineState);
    break;

  case e_JavaOpCodes::IntegerAND:
    ExecuteOpCodeANDInteger(pVirtualMachineState);
    break;

  case e_JavaOpCodes::DuplicateTopOperandOrTwo:
    ExecuteOpCodeDuplicateTopOperandOrTwo(pVirtualMachineState);
    break;

  case e_JavaOpCodes::ShiftIntegerRightLogical:
    ExecuteOpCodeShiftIntegerRightLogical(pVirtualMachineState);
    break;

  case e_JavaOpCodes::LoadIntegerFromArray:
    ExecuteOpCodeLoadIntegerFromArray(pVirtualMachineState);
    break;

  case e_JavaOpCodes::LoadLongFromArray:
    ExecuteOpCodeLoadLongFromArray(pVirtualMachineState);
    break;

  case e_JavaOpCodes::IntegerOR:
    ExecuteOpCodeORInteger(pVirtualMachineState);
    break;

  case e_JavaOpCodes::StoreIntoFloatArray:
    ExecuteOpCodeStoreIntoFloatArray(pVirtualMachineState);
    break;

  case e_JavaOpCodes::LoadFloatFromArray:
    ExecuteOpCodeLoadFloatFromArray(pVirtualMachineState);
    break;

  case e_JavaOpCodes::ConvertFloatToDouble:
    ExecuteOpCodeConvertFloatToDouble(pVirtualMachineState);
    break;

  case e_JavaOpCodes::DoubleAdd:
    ExecuteOpCodeDoubleAdd(pVirtualMachineState);
    break;

  case e_JavaOpCodes::DoubleMultiply:
    ExecuteOpCodeDoubleMultiply(pVirtualMachineState);
    break;

  case e_JavaOpCodes::ConvertDoubleToFloat:
    ExecuteOpCodeConvertDoubleToFloat(pVirtualMachineState);
    break;

  case e_JavaOpCodes::FloatDivide:
    ExecuteOpCodeFloatDivide(pVirtualMachineState);
    break;

  case e_JavaOpCodes::ConvertDoubleToInt:
    ExecuteOpCodeConvertDoubleToInt(pVirtualMachineState);
    break;

  case e_JavaOpCodes::StoreFloatInLocal:
    ExecuteOpCodeStoreFloatInLocalWithIndex(pVirtualMachineState);
    break;

  case e_JavaOpCodes::StoreFloatInLocal_0:
    ExecuteOpCodeStoreFloatInLocal(pVirtualMachineState, 0);
    break;

  case e_JavaOpCodes::StoreFloatInLocal_1:
    ExecuteOpCodeStoreFloatInLocal(pVirtualMachineState, 1);
    break;

  case e_JavaOpCodes::StoreFloatInLocal_2:
    ExecuteOpCodeStoreFloatInLocal(pVirtualMachineState, 2);
    break;

  case e_JavaOpCodes::StoreFloatInLocal_3:
    ExecuteOpCodeStoreFloatInLocal(pVirtualMachineState, 3);
    break;

  case e_JavaOpCodes::FloatSubtract:
    ExecuteOpCodeFloatSubtract(pVirtualMachineState);
    break;

  case e_JavaOpCodes::PushDouble_0:
    ExecuteOpCodePushDouble(pVirtualMachineState, 0.0);
    break;

  case e_JavaOpCodes::PushDouble_1:
    ExecuteOpCodePushDouble(pVirtualMachineState, 1.0);
    break;

  case e_JavaOpCodes::DoubleComparisonG:
    ExecuteOpCodeDoubleComparisonG(pVirtualMachineState);
    break;

  case e_JavaOpCodes::DoubleComparisonL:
    ExecuteOpCodeDoubleComparisonL(pVirtualMachineState);
    break;

  case e_JavaOpCodes::FloatAdd:
    ExecuteOpCodeFloatAdd(pVirtualMachineState);
    break;

  case e_JavaOpCodes::DoubleSubtract:
    ExecuteOpCodeDoubleSubtract(pVirtualMachineState);
    break;

  case e_JavaOpCodes::DuplicateTopOperandx2:
    ExecuteOpCodeDuplicateTopOperandx2(pVirtualMachineState);
    break;

  case e_JavaOpCodes::ConvertIntegerToShort:
    ExecuteOpCodeConvertIntegerToShort(pVirtualMachineState);
    break;

  case e_JavaOpCodes::DuplicateTopOperandOrTwox1:
    ExecuteOpCodeDuplicateTopOperandOrTwox1(pVirtualMachineState);
    break;

  case e_JavaOpCodes::LookupSwitch:
    ExecuteOpCodeLookupSwitch(pVirtualMachineState);
    break;

  case e_JavaOpCodes::StoreIntoDoubleArray:
    ExecuteOpCodeStoreIntoDoubleArray(pVirtualMachineState);
    break;

  case e_JavaOpCodes::LoadDoubleFromArray:
    ExecuteOpCodeLoadDoubleFromArray(pVirtualMachineState);
    break;

  case e_JavaOpCodes::LongOr:
    ExecuteOpCodeORLong(pVirtualMachineState);
    break;

  case e_JavaOpCodes::PopOperandStack_2:
    ExecuteOpCodePopOperandStack_2(pVirtualMachineState);
    break;

  case e_JavaOpCodes::DuplicateTopOperandOrTwox2:
    ExecuteOpCodeDuplicateTopOperandOrTwox2(pVirtualMachineState);
    break;

  case e_JavaOpCodes::SwapOperands:
    ExecuteOpCodeSwapOperands(pVirtualMachineState);
    break;

  case e_JavaOpCodes::NegateLong:
    ExecuteOpCodeNegateLong(pVirtualMachineState);
    break;

  case e_JavaOpCodes::NegateFloat:
    ExecuteOpCodeNegateFloat(pVirtualMachineState);
    break;

  case e_JavaOpCodes::ShiftLongRightArithmetic:
    ExecuteOpCodeShiftLongRightArithmetic(pVirtualMachineState);
    break;

  case e_JavaOpCodes::ConvertLongToFloat:
    ExecuteOpCodeConvertLongToFloat(pVirtualMachineState);
    break;

  case e_JavaOpCodes::ConvertFloatToLong:
    ExecuteOpCodeConvertFloatToLong(pVirtualMachineState);
    break;

  case e_JavaOpCodes::ConvertDoubleToLong:
    ExecuteOpCodeConvertDoubleToLong(pVirtualMachineState);
    break;

  case e_JavaOpCodes::GotoWide:
    ExecuteOpCodeGotoWide(pVirtualMachineState);
    break;

  case e_JavaOpCodes::JumpSubRoutineWide:
    ExecuteOpCodeJumpSubRoutineWide(pVirtualMachineState);
    break;

  case e_JavaOpCodes::InvokeDynamic:
    // Resolve the constant-pool entry and BootstrapMethods arguments, then link and cache the resulting call site.
  default:
    pLogger->LogDebug("%s %Id - Read opcode: %hu (%s)\n", pVirtualMachineState->GetCurrentClassAndMethodName().ToUtf8String().c_str(), pVirtualMachineState->GetProgramCounter() - 1, opCode, TranslateOpCode(opCode));
    pVirtualMachineState->LogCallStack();
    JVMX_ASSERT(false);
    break;
  }

  return e_ImmediateReturnRequired::No;
}

uint16_t BasicExecutionEngine::GetNextInstruction(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  uint16_t result = pVirtualMachineState->GetCodeSegmentStart()[pVirtualMachineState->GetProgramCounter()];
  pVirtualMachineState->AdvanceProgramCounter(1);

  return result;
}

ConstantPoolIndex BasicExecutionEngine::ReadIndex(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  if (!pVirtualMachineState->CanReadBytes(sizeof(ConstantPoolIndex)))
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Not enough space in current code segment to read an index." ));
  }

  uint8_t indexByte1 = pVirtualMachineState->GetCodeSegmentStart()[pVirtualMachineState->GetProgramCounter()];
  pVirtualMachineState->AdvanceProgramCounter(1);
  uint8_t indexByte2 = pVirtualMachineState->GetCodeSegmentStart()[pVirtualMachineState->GetProgramCounter()];
  pVirtualMachineState->AdvanceProgramCounter(1);

  return (indexByte1 << 8) | indexByte2;
}

const char* BasicExecutionEngine::TranslateOpCode(uint16_t opcode)
{
  switch ((e_JavaOpCodes)opcode)
  {
  case e_JavaOpCodes::PushNull:
    return "aconst_null";
    break;

  case e_JavaOpCodes::PushInt_Minus1:
    return "iconst_m1";
    break;

  case e_JavaOpCodes::PushInt_0:
    return "iconst_0";
    break;

  case e_JavaOpCodes::PushInt_1:
    return "iconst_1";
    break;

  case e_JavaOpCodes::PushInt_2:
    return "iconst_2";
    break;

  case e_JavaOpCodes::PushInt_3:
    return "iconst_3";
    break;

  case e_JavaOpCodes::PushInt_4:
    return "iconst_4";
    break;

  case e_JavaOpCodes::PushInt_5:
    return "iconst_5";
    break;

  case e_JavaOpCodes::PushLong_0:
    return "lconst_0";
    break;

  case e_JavaOpCodes::PushLong_1:
    return "lconst_1";
    break;

  case e_JavaOpCodes::PushInt_ImmediateByte:
    return "bipush";
    break;

  case e_JavaOpCodes::PushFromConstantPoolWide:
    return "ldc_w";
    break;

  case e_JavaOpCodes::PushDoubleOrLongFromConstantPoolWide:
    return "ldc2_w";
    break;

  case e_JavaOpCodes::PushFloatConstant_0:
    return "fconst_0";
    break;

  case e_JavaOpCodes::PushFloatConstant_1:
    return "fconst_1";
    break;

  case e_JavaOpCodes::PushFloatConstant_2:
    return "fconst_2";
    break;

  case e_JavaOpCodes::PushShortConstant:
    return "sipush";
    break;

  case e_JavaOpCodes::LoadReferenceFromArray:
    return "aaload";
    break;

  case e_JavaOpCodes::LoadCharacterFromArray:
    return "caload";
    break;

  case e_JavaOpCodes::LoadShortFromArray:
    return "saload";
    break;

  case e_JavaOpCodes::StoreLongInLocal:
    return "lstore";
    break;

  case e_JavaOpCodes::StoreLongInLocal_0:
    return "lstore_0";
    break;

  case e_JavaOpCodes::StoreLongInLocal_1:
    return "lstore_1";
    break;

  case e_JavaOpCodes::StoreLongInLocal_2:
    return "lstore_2";
    break;

  case e_JavaOpCodes::StoreLongInLocal_3:
    return "lstore_3";
    break;

  case e_JavaOpCodes::StoreReferenceInLocal:
    return "astore";
    break;

  case e_JavaOpCodes::StoreReferenceInLocal_0:
    return "astore_0";
    break;

  case e_JavaOpCodes::StoreReferenceInLocal_1:
    return "astore_1";
    break;

  case e_JavaOpCodes::StoreReferenceInLocal_2:
    return "astore_2";
    break;

  case e_JavaOpCodes::StoreReferenceInLocal_3:
    return "astore_3";
    break;

  case e_JavaOpCodes::GetStatic:
    return "getstatic";
    break;

  case e_JavaOpCodes::PutStatic:
    return "putstatic";
    break;

  case e_JavaOpCodes::GetField:
    return "getfield";
    break;

  case e_JavaOpCodes::PutField:
    return "putfield";
    break;

  case e_JavaOpCodes::LoadReferenceFromLocal:
    return "aload_index";
    break;

  case e_JavaOpCodes::LoadReferenceFromLocal_0:
    return "aload_0";
    break;

  case e_JavaOpCodes::LoadReferenceFromLocal_1:
    return "aload_1";
    break;

  case e_JavaOpCodes::LoadReferenceFromLocal_2:
    return "aload_2";
    break;

  case e_JavaOpCodes::LoadReferenceFromLocal_3:
    return "aload_3";
    break;

  case e_JavaOpCodes::StoreIntegerInLocal:
    return "istore";
    break;

  case e_JavaOpCodes::StoreIntegerInLocal_0:
    return "istore_0";
    break;

  case e_JavaOpCodes::StoreIntegerInLocal_1:
    return "istore_1";
    break;

  case e_JavaOpCodes::StoreIntegerInLocal_2:
    return "istore_2";
    break;

  case e_JavaOpCodes::StoreIntegerInLocal_3:
    return "istore_3";
    break;

  case e_JavaOpCodes::StoreDoubleInLocal:
    return "dstore";
    break;

  case e_JavaOpCodes::StoreDoubleInLocal_0:
    return "dstore_0";
    break;

  case e_JavaOpCodes::StoreDoubleInLocal_1:
    return "dstore_1";
    break;

  case e_JavaOpCodes::StoreDoubleInLocal_2:
    return "dstore_2";
    break;

  case e_JavaOpCodes::StoreDoubleInLocal_3:
    return "dstore_3";
    break;

  case e_JavaOpCodes::InvokeStatic:
    return "invokestatic";
    break;

  case e_JavaOpCodes::InvokeInterface:
    return "invokeinterface";
    break;

  case e_JavaOpCodes::InvokeSpecial:
    return "invokespecial";
    break;

  case e_JavaOpCodes::ReturnVoid:
    return "vreturn";
    break;

  case e_JavaOpCodes::ReturnReference:
    return "areturn";
    break;

  case e_JavaOpCodes::StoreIntoIntArray:
    return "iastore";
    break;

  case e_JavaOpCodes::StoreIntoLongArray:
    return "lastore";
    break;

  case e_JavaOpCodes::DuplicateTopOperand:
    return "dup";
    break;

  case e_JavaOpCodes::DuplicateTopOperandOrTwo:
    return "dup2";
    break;

  case e_JavaOpCodes::DuplicateTopOperandx1:
    return "dup_x1";
    break;

  case e_JavaOpCodes::PopOperandStack:
    return"pop";
    break;

  case e_JavaOpCodes::PopOperandStack_2:
    return "pop2";
    break;

  case e_JavaOpCodes::StoreIntoCharArray:
    return "castore";
    break;

  case e_JavaOpCodes::StoreIntoShortArray:
    return "sastore";
    break;

  case e_JavaOpCodes::StoreIntoByteArray:
    return "bastore";
    break;

  case e_JavaOpCodes::LoadByteOrBooleanFromArray:
    return "baload";
    break;

  case e_JavaOpCodes::StoreIntoReferenceArray:
    return "aastore";
    break;

  case e_JavaOpCodes::IntegerAdd:
    return "iadd";
    break;

  case e_JavaOpCodes::IntegerXOR:
    return "ixor";
    break;

  case e_JavaOpCodes::ThrowReference:
    return "athrow";
    break;

  case e_JavaOpCodes::LongComparison:
    return "lcmp";
    break;

  case e_JavaOpCodes::IntegerSubtract:
    return "isub";
    break;

  case e_JavaOpCodes::LongSubtract:
    return "lsub";
    break;

  case e_JavaOpCodes::IntegerMultiply:
    return "imul";
    break;

  case e_JavaOpCodes::LongMultiply:
    return "lmul";
    break;

  case e_JavaOpCodes::LongAdd:
    return "ladd";
    break;

  case e_JavaOpCodes::FloatMultiply:
    return "fmul";
    break;

  case e_JavaOpCodes::IntegerDivide:
    return "idiv";
    break;

  case e_JavaOpCodes::LongDivide:
    return "ldiv";
    break;

  case e_JavaOpCodes::IntegerRemainder:
    return "irem";
    break;

  case e_JavaOpCodes::LongRemainder:
    return "lrem";
    break;

  case e_JavaOpCodes::FloatRemainder:
    return "frem";
    break;

  case e_JavaOpCodes::DoubleRemainder:
    return "drem";
    break;

  case e_JavaOpCodes::NegateInteger:
    return "ineg";
    break;

  case e_JavaOpCodes::ShiftIntegerLeft:
    return "ishl";
    break;

  case e_JavaOpCodes::ShiftIntegerRightArithmetic:
    return "ishr";
    break;

  case e_JavaOpCodes::ShiftIntegerRightLogical:
    return "iushr";
    break;

  case e_JavaOpCodes::IncrementLocalVariable:
    return "iinc";
    break;

  case e_JavaOpCodes::ConvertIntegerToFloat:
    return "i2f";
    break;

  case e_JavaOpCodes::ConvertIntegerToDouble:
    return "i2d";
    break;

  case e_JavaOpCodes::ConvertFloatToInteger:
    return "f2i";
    break;

  case e_JavaOpCodes::ConvertIntegerToChar:
    return "i2c";
    break;

  case e_JavaOpCodes::FloatingPointComparisonL:
    return "fcmpl";
    break;

  case e_JavaOpCodes::FloatingPointComparisonG:
    return "fcmpg";
    break;

  case e_JavaOpCodes::New:
    return "new";
    break;

  case e_JavaOpCodes::NewArrayOfReference:
    return "anewarray";
    break;

  case e_JavaOpCodes::ArrayLength:
    return "arraylength";
    break;

  case e_JavaOpCodes::IsInstanceOf:
    return "instanceof";
    break;

  case e_JavaOpCodes::MonitorEnter:
    return "monitorenter";
    break;

  case e_JavaOpCodes::MonitorExit:
    return "monitorexit";
    break;

  case e_JavaOpCodes::Wide:
    return "wide";
    break;

  case e_JavaOpCodes::CheckCast:
    return "checkcast";
    break;

  case e_JavaOpCodes::NewArray:
    return "newarray";
    break;

  case e_JavaOpCodes::NewMultiDimentionalArray:
    return "NewMultiDimentionalArray";
    break;

  case e_JavaOpCodes::LoadReferenceFromConstantPool:
    return "ldc";
    break;

  case e_JavaOpCodes::InvokeVirtual:
    return "invokevirtual";
    break;

  case e_JavaOpCodes::BranchIfEquals:
    return "ifeq";
    break;

  case e_JavaOpCodes::BranchIfNotEquals:
    return "ifne";
    break;

  case e_JavaOpCodes::BranchIfLessThan:
    return "iflt";
    break;

  case e_JavaOpCodes::BranchIfGreaterThanEquals:
    return "ifge";
    break;

  case e_JavaOpCodes::BranchIfGreaterThan:
    return "ifgt";
    break;

  case e_JavaOpCodes::BranchIfLessThanEquals:
    return "ifle";
    break;

  case e_JavaOpCodes::BranchIfIntegerEquals:
    return "if_icmpeq";
    break;

  case e_JavaOpCodes::BranchIfIntegerNotEquals:
    return "if_icmpne";
    break;

  case e_JavaOpCodes::BranchIfIntegerLessThan:
    return "if_icmplt";
    break;

  case e_JavaOpCodes::BranchIfIntegerGreaterThanEquals:
    return "if_icmpge";
    break;

  case e_JavaOpCodes::BranchIfIntegerGreaterThan:
    return "if_icmpgt";
    break;

  case e_JavaOpCodes::BranchIfIntegerLessThanEquals:
    return "if_icmple";
    break;

  case e_JavaOpCodes::BranchIfReferencesAreEqual:
    return "if_acmpeq";
    break;

  case e_JavaOpCodes::BranchIfReferencesAreNotEqual:
    return "if_acmpne";
    break;

  case e_JavaOpCodes::LoadIntegerFromLocal:
    return "iload";
    break;

  case e_JavaOpCodes::LoadLongFromLocal:
    return "lload";
    break;

  case e_JavaOpCodes::LoadIntegerFromLocal_0:
    return "iload_0";
    break;

  case e_JavaOpCodes::LoadIntegerFromLocal_1:
    return "iload_1";
    break;

  case e_JavaOpCodes::LoadIntegerFromLocal_2:
    return "iload_2";
    break;

  case e_JavaOpCodes::LoadIntegerFromLocal_3:
    return "iload_3";
    break;

  case e_JavaOpCodes::LoadLongFromLocal_0:
    return "lload_0";
    break;

  case e_JavaOpCodes::LoadLongFromLocal_1:
    return "lload_1";
    break;

  case e_JavaOpCodes::LoadLongFromLocal_2:
    return "lload_2";
    break;

  case e_JavaOpCodes::LoadLongFromLocal_3:
    return "lload_3";
    break;

  case e_JavaOpCodes::LoadFloatFromLocal:
    return "fload";
    break;

  case e_JavaOpCodes::LoadFloatFromLocal_0:
    return "fload_0";
    break;

  case e_JavaOpCodes::LoadFloatFromLocal_1:
    return "fload_1";
    break;

  case e_JavaOpCodes::LoadFloatFromLocal_2:
    return "fload_2";
    break;

  case e_JavaOpCodes::LoadFloatFromLocal_3:
    return "fload_3";
    break;

  case e_JavaOpCodes::LoadDoubleFromLocal:
    return "dload";
    break;

  case e_JavaOpCodes::LoadDoubleFromLocal_0:
    return "dload_0";
    break;

  case e_JavaOpCodes::LoadDoubleFromLocal_1:
    return "dload_1";
    break;

  case e_JavaOpCodes::LoadDoubleFromLocal_2:
    return "dload_2";
    break;

  case e_JavaOpCodes::LoadDoubleFromLocal_3:
    return "dload_3";
    break;

  case e_JavaOpCodes::Goto:
    return "goto";
    break;

  case e_JavaOpCodes::JumpSubRoutine:
    return "jsr";
    break;

  case e_JavaOpCodes::ReturnFromSubRoutine:
    return "ret";
    break;

  case e_JavaOpCodes::TableSwitch:
    return "tableswitch";
    break;

  case e_JavaOpCodes::ReturnInteger:
    return "ireturn";
    break;

  case e_JavaOpCodes::BranchIfNotNull:
    return "ifnonnull";
    break;

  case e_JavaOpCodes::BranchIfNull:
    return "ifnull";
    break;

  case e_JavaOpCodes::ConvertIntegerToByte:
    return "i2b";
    break;

  case e_JavaOpCodes::LongXOR:
    return "lxor";
    break;

  case e_JavaOpCodes::LongAND:
    return "land";
    break;

  case e_JavaOpCodes::ShiftLongRight:
    return "lushr";
    break;

  case e_JavaOpCodes::ShiftLongLeft:
    return "lushl";
    break;

  case e_JavaOpCodes::ConvertLongToInteger:
    return "l2i";
    break;

  case e_JavaOpCodes::ConvertIntegerToLong:
    return "i2l";
    break;

  case e_JavaOpCodes::ConvertLongToDouble:
    return "l2d";
    break;

  case e_JavaOpCodes::NoOperation:
    return "nop";
    break;

  case e_JavaOpCodes::NegateDouble:
    return "dneg";
    break;

  case e_JavaOpCodes::IntegerAND:
    return "iand";
    break;

  case e_JavaOpCodes::IntegerOR:
    return "ior";
    break;

  case e_JavaOpCodes::ReturnFloat:
    return "freturn";
    break;

  case e_JavaOpCodes::ReturnDouble:
    return "dreturn";
    break;

  case e_JavaOpCodes::LoadIntegerFromArray:
    return "iaload";
    break;

  case e_JavaOpCodes::LoadLongFromArray:
    return "laload";
    break;

  case e_JavaOpCodes::StoreIntoFloatArray:
    return "fastore";
    break;

  case e_JavaOpCodes::LoadFloatFromArray:
    return "faload";
    break;

  case e_JavaOpCodes::ConvertFloatToDouble:
    return "f2d";
    break;

  case e_JavaOpCodes::DoubleAdd:
    return "dadd";
    break;

  case e_JavaOpCodes::DoubleMultiply:
    return "dmul";
    break;

  case e_JavaOpCodes::ConvertDoubleToFloat:
    return "d2f";
    break;

  case e_JavaOpCodes::FloatDivide:
    return "fdiv";
    break;

  case e_JavaOpCodes::ConvertDoubleToInt:
    return "d2i";
    break;

  case e_JavaOpCodes::StoreFloatInLocal:
    return "fstore";
    break;

  case e_JavaOpCodes::StoreFloatInLocal_0:
    return "fstore_0";
    break;

  case e_JavaOpCodes::StoreFloatInLocal_1:
    return "fstore_1";
    break;

  case e_JavaOpCodes::StoreFloatInLocal_2:
    return "fstore_2";
    break;

  case e_JavaOpCodes::StoreFloatInLocal_3:
    return "fstore_3";
    break;

  case e_JavaOpCodes::PushDouble_0:
    return "dcost_0";
    break;

  case e_JavaOpCodes::PushDouble_1:
    return "dcost_1";
    break;

  case e_JavaOpCodes::DoubleComparisonL:
    return "dcmpl";
    break;

  case e_JavaOpCodes::DoubleComparisonG:
    return "dcmpg";
    break;

  case e_JavaOpCodes::FloatAdd:
    return "fadd";
    break;

  case e_JavaOpCodes::DoubleSubtract:
    return "dsub";
    break;

  case e_JavaOpCodes::DuplicateTopOperandx2:
    return "dup_x2";
    break;

  case e_JavaOpCodes::ConvertIntegerToShort:
    return "i2s";
    break;

  case e_JavaOpCodes::DuplicateTopOperandOrTwox1:
    return "dup2_x1";
    break;

  case e_JavaOpCodes::LookupSwitch:
    return"lookupswitch";
    break;

  case e_JavaOpCodes::StoreIntoDoubleArray:
    return "dastore";
    break;

  case e_JavaOpCodes::LoadDoubleFromArray:
    return "daload";
    break;

  case e_JavaOpCodes::ReturnLong:
    return "lreturn";

  case e_JavaOpCodes::DuplicateTopOperandOrTwox2:
    return "dup2_x2";
    break;

  case e_JavaOpCodes::SwapOperands:
    return "swap";
    break;

  case e_JavaOpCodes::NegateLong:
    return "lneg";
    break;

  case e_JavaOpCodes::NegateFloat:
    return "fneg";
    break;

  case e_JavaOpCodes::ShiftLongRightArithmetic:
    return "lshr";
    break;

  case e_JavaOpCodes::ConvertLongToFloat:
    return "l2f";
    break;

  case e_JavaOpCodes::ConvertFloatToLong:
    return "f2l";
    break;

  case e_JavaOpCodes::ConvertDoubleToLong:
    return "d2l";
    break;

  case e_JavaOpCodes::InvokeDynamic:
    return "invokedynamic";
    break;

  case e_JavaOpCodes::GotoWide:
    return "goto_w";
    break;

  case e_JavaOpCodes::JumpSubRoutineWide:
    return "jsr_w";
    break;

  default:
    return "Unknown";
    break;
  }
}

boost::intrusive_ptr<ObjectReference> GetFromStringPool(std::shared_ptr<ConstantPoolStringReference> pStringRef)
{
  std::shared_ptr<StringPool> pStringPool = GlobalCatalog::GetInstance().Get("StringPool");
  auto pObject = pStringPool->Get(*(pStringRef->GetStringValue()));
  return pObject;
}

void AddToStringPool(std::shared_ptr<ConstantPoolStringReference> pStringRef, boost::intrusive_ptr<ObjectReference> pObject)
{
  std::shared_ptr<StringPool> pStringPool = GlobalCatalog::GetInstance().Get("StringPool");
  pStringPool->Add(*(pStringRef->GetStringValue()), pObject);
}

// uint8_t BasicExecutionEngine::ReadByte( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState )
// {
//   if ( !pVirtualMachineState->CanReadBytes( 1 ) )
//   {
//     throw InvalidStateException( JVMX_FUNCTION_MESSAGE( " - Should be able to read 1 bytes for this instruction, but the code segment is too short." ) );
//   }
//
//   uint8_t result = pVirtualMachineState->GetCodeSegmentStart()[ pVirtualMachineState->GetProgramCounter() ];
//   pVirtualMachineState->AdvanceProgramCounter( 1 );
//
//   return result;
// }

int8_t BasicExecutionEngine::ReadByteSigned(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  if (!pVirtualMachineState->CanReadBytes(1))
  {
    pVirtualMachineState->LogCallStack();
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Should be able to read 1 bytes for this instruction, but the code segment is too short." ));
  }

  int8_t result = pVirtualMachineState->GetCodeSegmentStart()[pVirtualMachineState->GetProgramCounter()];
  pVirtualMachineState->AdvanceProgramCounter(1);

  return result;
}

int16_t BasicExecutionEngine::ReadOffset(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  if (!pVirtualMachineState->CanReadBytes(sizeof(ConstantPoolIndex)))
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Not enough space in current code segment to read an index." ));
  }

  uint8_t branchByte1 = pVirtualMachineState->GetCodeSegmentStart()[pVirtualMachineState->GetProgramCounter()];
  pVirtualMachineState->AdvanceProgramCounter(1);
  uint8_t branchByte2 = pVirtualMachineState->GetCodeSegmentStart()[pVirtualMachineState->GetProgramCounter()];
  pVirtualMachineState->AdvanceProgramCounter(1);

  // #ifdef _DEBUG
  //   if ( ((branchByte1 << 8) | branchByte2) == -124 )
  //   {
  //     BreakDebug(__FUNCTION__, "Test" );
  //   }
  // #endif // _DEBUG

  return static_cast<int16_t>((branchByte1 << 8) | branchByte2);
}

#ifdef _DEBUG
void BasicExecutionEngine::BreakDebug(const char* pString1, const char* pString2)
{
  if (strcmp(pString1, pString2) == 0)
  {
    JvmxDebugBreak();
  }
}

void BasicExecutionEngine::BreakDebug(const char16_t* pString1, const char16_t* pString2)
{
  if (wcscmp((const wchar_t*)pString1, (const wchar_t*)pString2) == 0)
  {
    JvmxDebugBreak();
  }
}

void BasicExecutionEngine::BreakDebug(const uint8_t* pString1, const char* pString2)
{
  BreakDebug((const char*)pString1, (const char*)pString2);
}

void BasicExecutionEngine::BreakDebug(const uint8_t* pString1, const uint8_t* pString2)
{
  BreakDebug((const char*)pString1, (const char*)pString2);
}
#endif // _DEBUG

uint8_t BasicExecutionEngine::ReadByteUnsigned(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  return static_cast<uint8_t>(ReadByteSigned(pVirtualMachineState));
}

int32_t BasicExecutionEngine::Read32BitOffset(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  uint8_t byte1 = ReadByteUnsigned(pVirtualMachineState);
  uint8_t byte2 = ReadByteUnsigned(pVirtualMachineState);
  uint8_t byte3 = ReadByteUnsigned(pVirtualMachineState);
  uint8_t byte4 = ReadByteUnsigned(pVirtualMachineState);

  uint32_t bits = (static_cast<uint32_t>(byte1) << 24) |
    (static_cast<uint32_t>(byte2) << 16) |
    (static_cast<uint32_t>(byte3) << 8) |
    static_cast<uint32_t>(byte4);

  if ((bits & 0x80000000u) != 0)
  {
    return -1 - static_cast<int32_t>(~bits);
  }

  return static_cast<int32_t>(bits);
}

void BasicExecutionEngine::Halt()
{
  m_Halted = true;
}

std::shared_ptr<IClassLibrary> BasicExecutionEngine::GetClassLibrary() const
{
  return GlobalCatalog::GetInstance().Get("ClassLibrary");
}

std::shared_ptr<ILogger> BasicExecutionEngine::GetLogger() const
{
  return GlobalCatalog::GetInstance().Get("Logger");
}

int16_t BasicExecutionEngine::ReadShort(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  if (!pVirtualMachineState->CanReadBytes(sizeof(ConstantPoolIndex)))
  {
    throw InvalidStateException(JVMX_FUNCTION_MESSAGE( " - Not enough space in current code segment to read a short." ));
  }

  int8_t branchByte1 = pVirtualMachineState->GetCodeSegmentStart()[pVirtualMachineState->GetProgramCounter()];
  pVirtualMachineState->AdvanceProgramCounter(1);
  uint8_t branchByte2 = pVirtualMachineState->GetCodeSegmentStart()[pVirtualMachineState->GetProgramCounter()];
  pVirtualMachineState->AdvanceProgramCounter(1);

  return (branchByte1 << 8) | branchByte2;
}
