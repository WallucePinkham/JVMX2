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

#if 1
#define __EXRTA_LOGGING
#endif

extern const JavaString c_ClassInitialisationMethodName;
extern const JavaString c_InstanceInitialisationMethodName;

const JavaString c_MethodTypeClassName = JavaString::FromCString( JVMX_T( "java/lang/invoke/MethodType" ) );
const JavaString c_ThrowableClassName = JavaString::FromCString( JVMX_T( "java/lang/Throwable" ) );

const JavaString c_UncaughtExceptionMethodName = JavaString::FromCString(JVMX_T("uncaughtException"));
const JavaString c_UncaughtExceptionMethodType = JavaString::FromCString(JVMX_T("(Ljava/lang/Thread;Ljava/lang/Throwable;)V"));

class StackLevelIncrementer
{
public:
  explicit StackLevelIncrementer( std::atomic_int64_t &level )
    : m_Level( level )
  {
    ++m_Level;
  }

  ~StackLevelIncrementer()
  {
    --m_Level;
    JVMX_ASSERT( m_Level >= 0 );
  }

private:
  std::atomic_int64_t &m_Level;
};

BasicExecutionEngine::BasicExecutionEngine()
  : m_Halted( false )
{
#ifdef _DEBUG
  m_InstructionsExecuted = 0;
#endif // _DEBUG
}

BasicExecutionEngine::~BasicExecutionEngine()
{}

void BasicExecutionEngine::Run( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  StackLevelIncrementer incrementer( pVirtualMachineState->GetStackLevel() );
  intptr_t savedProgramCounter = 0; // Save the program counter before the next instruction is executed, for exception handling.

  std::shared_ptr<IGarbageCollector> pGarbageCollector = GlobalCatalog::GetInstance().Get( "GarbageCollector" );
  pVirtualMachineState->PushAndZeroCallStackDepth();

  while ( !m_Halted && pVirtualMachineState->GetProgramCounter() < pVirtualMachineState->GetCodeSegmentLength() )
  {
    if ( pVirtualMachineState->IsPausing() )
    {
      pVirtualMachineState->ConfirmPaused();
    }

    if ( pVirtualMachineState->IsPaused() )
    {
      std::this_thread::yield();
      continue;
    }

    TryDoGarbageCollection( pVirtualMachineState, pGarbageCollector );

    if ( !pVirtualMachineState->HasExceptionOccurred() )
    {
      if ( e_ImmediateReturnRequired::Yes == ProcessNextOpcode( pVirtualMachineState, GetLogger() ) )
      {
        return;
      }
    }
    // Don't put an else here, because ProcessNextOpCode will set the flag to indicate an Exception has occurred.

    if ( pVirtualMachineState->HasExceptionOccurred() )
    {
      savedProgramCounter = pVirtualMachineState->GetProgramCounter() - 1;

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
      GetLogger()->LogDebug( "Exception occurred in %s (%s) : %lld", pVirtualMachineState->GetCurrentClassAndMethodName().ToUtf8String().c_str(), pVirtualMachineState->GetException()->GetContainedObject()->ToString().ToUtf8String().c_str(), savedProgramCounter );
#endif // _DEBUG

      if ( !CanExceptionBeHandledLocally( pVirtualMachineState, savedProgramCounter ) )
      {
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
        GetLogger()->LogDebug( "Exception cannot be handled in this function. Rewinding the stack, and executing a \"void\" return." );
#endif // _DEBUG

        RewindOperandStack( pVirtualMachineState, 0 );
        ExecuteOpCodeReturnVoid( pVirtualMachineState );

        if ( 0 == pVirtualMachineState->GetCallStackDepth() )
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
        CatchException( pVirtualMachineState, savedProgramCounter );
      }
    }
  }
}

void BasicExecutionEngine::TryDoGarbageCollection( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, const std::shared_ptr<IGarbageCollector> &pGarbageCollector )
{
#ifdef _DEBUG
  if ( m_InstructionsExecuted > 0 && 0 == ( m_InstructionsExecuted % 100000 ) )
  {
    GetLogger()->LogDebug( "%lld Instructions executed. MustCollect( %s ), StackLevel( %lld )", m_InstructionsExecuted, pGarbageCollector->MustCollect() ? "true" : "false", pVirtualMachineState->GetStackLevel().load() );
  }
#endif // _DEBUG

#ifdef _DEBUG
  if ( ( m_InstructionsExecuted > 0 && 0 == ( m_InstructionsExecuted % 100000 ) ) || pGarbageCollector->MustCollect() )
#else
  if ( pGarbageCollector->MustCollect() )
#endif // _DEBUG

  {

#ifdef JVMX_LOG_VERBOSE
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

e_ImmediateReturnRequired BasicExecutionEngine::ProcessNextOpcode( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, std::shared_ptr<ILogger> pLogger )
{
  uint16_t opCode = GetNextInstruction( pVirtualMachineState );

#ifdef JVMX_LOG_VERBOSE
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pLogger->LogDebug("%s %Id - Read opcode: %hu (%s)\n", pVirtualMachineState->GetCurrentClassAndMethodName().ToUtf8String().c_str(), pVirtualMachineState->GetProgramCounter() - 1, opCode, TranslateOpCode(opCode));
  }
#endif // _DEBUG

#if defined( _DEBUG ) && defined (__EXRTA_LOGGING)
  //BreakDebug( pVirtualMachineState->GetCurrentClassAndMethodName().ToUtf16String().c_str(), u"java/awt/color/ICC_Profile::createLinearRGBProfile" );
  static bool extraDebugLogging = false;
  static std::thread::id debugThreadId = std::thread::id();

  if ( pVirtualMachineState->GetCurrentClassAndMethodName().Contains( u"Level" ) )
  {
    extraDebugLogging = true;
    debugThreadId = std::this_thread::get_id();
  }

  if ( extraDebugLogging &&
       std::this_thread::get_id() == debugThreadId && (
         //pVirtualMachineState->GetCurrentClassAndMethodName().EndsWith( u"createProfile" ) ||
         //pVirtualMachineState->GetCurrentClassAndMethodName().EndsWith( u"setData" ) ||
         //pVirtualMachineState->GetCurrentClassAndMethodName().EndsWith( u"QtComponentPeer::updateBackBuffer" ) ||
         //pVirtualMachineState->GetCurrentClassAndMethodName().EndsWith( u"QtImage::createImage" ) ||
         /*( pVirtualMachineState->GetCurrentClassAndMethodName().EndsWith( u"Reference::get" )  && pVirtualMachineState->GetProgramCounter() >= 12 ) ||
         ( pVirtualMachineState->GetCurrentClassAndMethodName().EndsWith( u"AWTKeyStroke::<clinit>" ) && pVirtualMachineState->GetProgramCounter() >= 32 ) || /**/
         pVirtualMachineState->GetCurrentClassAndMethodName().EndsWith( u"parse" )
         //&& pVirtualMachineState->GetCurrentMethodType().EndsWith(u"m;)V")
       ) )
  {
    //if ( pVirtualMachineState->GetProgramCounter() >= 17 )
    {
      pVirtualMachineState->SetUserCodeStarted();
      pLogger->LogDebug( "@@Inside suspect method:" );
      pVirtualMachineState->LogLocalVariables();
      pVirtualMachineState->LogOperandStack();
      pVirtualMachineState->LogCallStack();
    }
  }

#if 0
  if ( extraDebugLogging && std::this_thread::get_id() == debugThreadId )
  {
    pLogger->LogDebug( "%s(%s) %Id - Read opcode: %hu (%s)\n", pVirtualMachineState->GetCurrentClassAndMethodName().ToUtf8String().c_str(), pVirtualMachineState->GetCurrentMethodType().ToUtf8String().c_str(), pVirtualMachineState->GetProgramCounter() - 1, opCode, TranslateOpCode( opCode ) );
    int i = 0;
  }
#endif 
#endif // _DEBUG


#ifdef _DEBUG
  ++m_InstructionsExecuted;
#endif // _DEBUG

  switch ( (e_JavaOpCodes)opCode )
  {
    case e_JavaOpCodes::PushNull:
      ExecuteOpCodePushNull( pVirtualMachineState );
      break;

    case e_JavaOpCodes::PushInt_Minus1:
      ExecuteOpCodePushInt( pVirtualMachineState, -1 );
      break;

    case e_JavaOpCodes::PushInt_0:
      ExecuteOpCodePushInt( pVirtualMachineState, 0 );
      break;

    case e_JavaOpCodes::PushInt_1:
      ExecuteOpCodePushInt( pVirtualMachineState, 1 );
      break;

    case e_JavaOpCodes::PushInt_2:
      ExecuteOpCodePushInt( pVirtualMachineState, 2 );
      break;

    case e_JavaOpCodes::PushInt_3:
      ExecuteOpCodePushInt( pVirtualMachineState, 3 );
      break;

    case e_JavaOpCodes::PushInt_4:
      ExecuteOpCodePushInt( pVirtualMachineState, 4 );
      break;

    case e_JavaOpCodes::PushInt_5:
      ExecuteOpCodePushInt( pVirtualMachineState, 5 );
      break;

    case e_JavaOpCodes::PushLong_0:
      ExecuteOpCodePushLong( pVirtualMachineState, 0 );
      break;

    case e_JavaOpCodes::PushLong_1:
      ExecuteOpCodePushLong( pVirtualMachineState, 1 );
      break;

    case e_JavaOpCodes::PushInt_ImmediateByte:
      ExecuteOpCodePushIntImmediateByte( pVirtualMachineState );
      break;

    case e_JavaOpCodes::PushFromConstantPoolWide:
      ExecuteOpCodePushFromConstantPoolWide( pVirtualMachineState );
      break;

    case e_JavaOpCodes::PushFloatConstant_0:
      ExecuteOpCodePushFloatConstant( pVirtualMachineState, 0.0f );
      break;

    case e_JavaOpCodes::PushFloatConstant_1:
      ExecuteOpCodePushFloatConstant( pVirtualMachineState, 1.0f );
      break;

    case e_JavaOpCodes::PushFloatConstant_2:
      ExecuteOpCodePushFloatConstant( pVirtualMachineState, 2.0f );
      break;

    case e_JavaOpCodes::PushShortConstant:
      ExecuteOpCodePushShortConstant( pVirtualMachineState );
      break;

    case e_JavaOpCodes::LoadReferenceFromArray:
      ExecuteOpCodeLoadReferenceFromArray( pVirtualMachineState );
      break;

    case e_JavaOpCodes::LoadCharacterFromArray:
      ExecuteOpCodeLoadCharacterFromArray( pVirtualMachineState );
      break;

    case e_JavaOpCodes::StoreLongInLocal:
      ExecuteOpCodeStoreLongInLocalWithIndex( pVirtualMachineState );
      break;

    case e_JavaOpCodes::StoreLongInLocal_0:
      ExecuteOpCodeStoreLongInLocal( pVirtualMachineState, 0 );
      break;

    case e_JavaOpCodes::StoreLongInLocal_1:
      ExecuteOpCodeStoreLongInLocal( pVirtualMachineState, 1 );
      break;

    case e_JavaOpCodes::StoreLongInLocal_2:
      ExecuteOpCodeStoreLongInLocal( pVirtualMachineState, 2 );
      break;

    case e_JavaOpCodes::StoreLongInLocal_3:
      ExecuteOpCodeStoreLongInLocal( pVirtualMachineState, 3 );
      break;

    case e_JavaOpCodes::StoreReferenceInLocal:
      ExecuteOpCodeStoreReferenceInLocalWithIndex( pVirtualMachineState );
      break;

    case e_JavaOpCodes::StoreReferenceInLocal_0:
      ExecuteOpCodeStoreReferenceInLocal( pVirtualMachineState, 0 );
      break;

    case e_JavaOpCodes::StoreReferenceInLocal_1:
      ExecuteOpCodeStoreReferenceInLocal( pVirtualMachineState, 1 );
      break;

    case e_JavaOpCodes::StoreReferenceInLocal_2:
      ExecuteOpCodeStoreReferenceInLocal( pVirtualMachineState, 2 );
      break;

    case e_JavaOpCodes::StoreReferenceInLocal_3:
      ExecuteOpCodeStoreReferenceInLocal( pVirtualMachineState, 3 );
      break;

    case e_JavaOpCodes::StoreIntegerInLocal:
      ExecuteOpCodeStoreIntegerInLocalWithIndex( pVirtualMachineState );
      break;

    case e_JavaOpCodes::StoreIntegerInLocal_0:
      ExecuteOpCodeStoreIntegerInLocal( pVirtualMachineState, 0 );
      break;

    case e_JavaOpCodes::StoreIntegerInLocal_1:
      ExecuteOpCodeStoreIntegerInLocal( pVirtualMachineState, 1 );
      break;

    case e_JavaOpCodes::StoreIntegerInLocal_2:
      ExecuteOpCodeStoreIntegerInLocal( pVirtualMachineState, 2 );
      break;

    case e_JavaOpCodes::StoreIntegerInLocal_3:
      ExecuteOpCodeStoreIntegerInLocal( pVirtualMachineState, 3 );
      break;

    case e_JavaOpCodes::LoadIntegerFromLocal:
      ExecuteOpCodeLoadIntegerFromLocalWithIndex( pVirtualMachineState );
      break;

    case e_JavaOpCodes::LoadLongFromLocal:
      ExecuteOpCodeLoadLongFromLocalWithIndex( pVirtualMachineState );
      break;

    case e_JavaOpCodes::LoadIntegerFromLocal_0:
      ExecuteOpCodeLoadIntegerFromLocal( pVirtualMachineState, 0 );
      break;

    case e_JavaOpCodes::LoadIntegerFromLocal_1:
      ExecuteOpCodeLoadIntegerFromLocal( pVirtualMachineState, 1 );
      break;

    case e_JavaOpCodes::LoadIntegerFromLocal_2:
      ExecuteOpCodeLoadIntegerFromLocal( pVirtualMachineState, 2 );
      break;

    case e_JavaOpCodes::LoadIntegerFromLocal_3:
      ExecuteOpCodeLoadIntegerFromLocal( pVirtualMachineState, 3 );
      break;

    case e_JavaOpCodes::LoadLongFromLocal_0:
      ExecuteOpCodeLoadLongFromLocal( pVirtualMachineState, 0 );
      break;

    case e_JavaOpCodes::LoadLongFromLocal_1:
      ExecuteOpCodeLoadLongFromLocal( pVirtualMachineState, 1 );
      break;

    case e_JavaOpCodes::LoadLongFromLocal_2:
      ExecuteOpCodeLoadLongFromLocal( pVirtualMachineState, 2 );
      break;

    case e_JavaOpCodes::LoadLongFromLocal_3:
      ExecuteOpCodeLoadLongFromLocal( pVirtualMachineState, 3 );
      break;

    case e_JavaOpCodes::LoadFloatFromLocal:
      ExecuteOpCodeLoadFloatFromLocalWithIndex( pVirtualMachineState );
      break;

    case e_JavaOpCodes::LoadFloatFromLocal_0:
      ExecuteOpLoadFloatFromLocal( pVirtualMachineState, 0 );
      break;

    case e_JavaOpCodes::LoadFloatFromLocal_1:
      ExecuteOpLoadFloatFromLocal( pVirtualMachineState, 1 );
      break;

    case e_JavaOpCodes::LoadFloatFromLocal_2:
      ExecuteOpLoadFloatFromLocal( pVirtualMachineState, 2 );
      break;

    case e_JavaOpCodes::LoadFloatFromLocal_3:
      ExecuteOpLoadFloatFromLocal( pVirtualMachineState, 3 );
      break;

    case e_JavaOpCodes::GetStatic:
      ExecuteOpCodeGetStatic( pVirtualMachineState );
      break;

    case e_JavaOpCodes::PutStatic:
      ExecuteOpCodePutStatic( pVirtualMachineState );
      break;

    case e_JavaOpCodes::GetField:
      ExecuteOpCodeGetField( pVirtualMachineState );
      break;

    case e_JavaOpCodes::PutField:
      ExecuteOpCodePutField( pVirtualMachineState );
      break;

    case e_JavaOpCodes::LoadReferenceFromLocal:
      ExecuteOpCodeLoadReferenceFromLocalWithSpecifiedIndex( pVirtualMachineState );
      break;

    case e_JavaOpCodes::LoadReferenceFromLocal_0:
      ExecuteOpCodeLoadReferenceFromLocalIndex( pVirtualMachineState, 0 );
      break;

    case e_JavaOpCodes::LoadReferenceFromLocal_1:
      ExecuteOpCodeLoadReferenceFromLocalIndex( pVirtualMachineState, 1 );
      break;

    case e_JavaOpCodes::LoadReferenceFromLocal_2:
      ExecuteOpCodeLoadReferenceFromLocalIndex( pVirtualMachineState, 2 );
      break;

    case e_JavaOpCodes::LoadReferenceFromLocal_3:
      ExecuteOpCodeLoadReferenceFromLocalIndex( pVirtualMachineState, 3 );
      break;

    case e_JavaOpCodes::InvokeVirtual:
      if ( e_IncreaseCallStackDepth::Yes == ExecuteOpCodeInvokeVirtual( pVirtualMachineState ) )
      {
        pVirtualMachineState->IncrementCallStackDepth();
      }
      break;

    case e_JavaOpCodes::InvokeSpecial:
      if ( e_IncreaseCallStackDepth::Yes == ExecuteOpCodeInvokeSpecial( pVirtualMachineState ) )
      {
        pVirtualMachineState->IncrementCallStackDepth();
      }
      break;

    case e_JavaOpCodes::InvokeStatic:
      if ( e_IncreaseCallStackDepth::Yes == ExecuteOpCodeInvokeStatic( pVirtualMachineState ) )
      {
        pVirtualMachineState->IncrementCallStackDepth();
      }
      break;

    case e_JavaOpCodes::InvokeInterface:
      if ( e_IncreaseCallStackDepth::Yes == ExecuteOpCodeInvokeInterfaceMethod( pVirtualMachineState ) )
      {
        pVirtualMachineState->IncrementCallStackDepth();
      }
      break;

    case e_JavaOpCodes::StoreIntoIntArray:
      ExecuteOpCodeStoreIntoIntArray( pVirtualMachineState );
      break;

    case e_JavaOpCodes::DuplicateTopOperand:
      ExecuteOpCodeDuplicateTopOperand( pVirtualMachineState );
      break;

    case e_JavaOpCodes::DuplicateTopOperandx1:
      ExecuteOpCodeDuplicateTopOperandx1( pVirtualMachineState );
      break;

    case e_JavaOpCodes::PopOperandStack:
      ExecuteOpCodePopOperandStack( pVirtualMachineState );
      break;

    case e_JavaOpCodes::StoreIntoCharArray:
      ExecuteOpCodeStoreIntoCharArray( pVirtualMachineState );
      break;

    case e_JavaOpCodes::StoreIntoReferenceArray:
      ExecuteOpCodeStoreIntoReferenceArray( pVirtualMachineState );
      break;

    case e_JavaOpCodes::IntegerAdd:
      ExecuteOpCodeIntegerAdd( pVirtualMachineState );
      break;

    case e_JavaOpCodes::IntegerSubtract:
      ExecuteOpCodeIntegerSubtract( pVirtualMachineState );
      break;

    case e_JavaOpCodes::LongSubtract:
      ExecuteOpCodeLongSubtract( pVirtualMachineState );
      break;

    case e_JavaOpCodes::IntegerMultiply:
      ExecuteOpCodeIntegerMultiply( pVirtualMachineState );
      break;

    case e_JavaOpCodes::FloatMultiply:
      ExecuteOpCodeFloatMultiply( pVirtualMachineState );
      break;

    case e_JavaOpCodes::IntegerDivide:
      ExecuteOpCodeIntegerDivide( pVirtualMachineState );
      break;

    case e_JavaOpCodes::IntegerRemainder:
      ExecuteOpCodeIntegerRemainder( pVirtualMachineState );
      break;

    case e_JavaOpCodes::NegateInteger:
      ExecuteOpCodeNegateInteger( pVirtualMachineState );
      break;

    case e_JavaOpCodes::ShiftIntegerLeft:
      ExecuteOpCodeShiftIntegerLeft( pVirtualMachineState );
      break;

    case e_JavaOpCodes::ShiftIntegerRightArithmetic:
      ExecuteOpCodeShiftIntegerRightArithmetic( pVirtualMachineState );
      break;

    case e_JavaOpCodes::IncrementLocalVariable:
      ExecuteOpCodeIncrementLocalVariable( pVirtualMachineState );
      break;

    case e_JavaOpCodes::ConvertIntegerToFloat:
      ExecuteOpCodeConvertIntegerToFloat( pVirtualMachineState );
      break;

    case e_JavaOpCodes::ConvertIntegerToDouble:
      ExecuteOpCodeConvertIntegerToDouble( pVirtualMachineState );
      break;

    case e_JavaOpCodes::ConvertFloatToInteger:
      ExecuteOpCodeConvertFloatToInteger( pVirtualMachineState );
      break;

    case e_JavaOpCodes::ConvertIntegerToChar:
      ExecuteOpCodeConvertIntegerToChar( pVirtualMachineState );
      break;

    case e_JavaOpCodes::FloatingPointComparisonL:
      ExecuteOpCodeFloatComparisonL( pVirtualMachineState );
      break;

    case e_JavaOpCodes::FloatingPointComparisonG:
      ExecuteOpCodeFloatComparisonG( pVirtualMachineState );
      break;

    case e_JavaOpCodes::ReturnVoid:
      {
        ExecuteOpCodeReturnVoid( pVirtualMachineState );

        if ( 0 == pVirtualMachineState->GetCallStackDepth() )
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
        ExecuteOpCodeReturnReference( pVirtualMachineState );

        if ( 0 == pVirtualMachineState->GetCallStackDepth() )
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
      ExecuteOpCodeNew( pVirtualMachineState );
      break;

    case e_JavaOpCodes::NewArray:
      ExecuteOpCodeNewArray( pVirtualMachineState );
      break;

    case e_JavaOpCodes::NewArrayOfReference:
      ExecuteOpCodeNewArrayOfReference( pVirtualMachineState );
      break;

    case e_JavaOpCodes::ArrayLength:
      ExecuteOpCodeArrayLength( pVirtualMachineState );
      break;

    case e_JavaOpCodes::IsInstanceOf:
      ExecuteOpCodeIsInstanceOf( pVirtualMachineState );
      break;

    case e_JavaOpCodes::MonitorEnter:
      ExecuteOpCodeMonitorEnter( pVirtualMachineState );
      break;

    case e_JavaOpCodes::MonitorExit:
      ExecuteOpCodeMonitorExit( pVirtualMachineState );
      break;

    case e_JavaOpCodes::CheckCast:
      ExecuteOpCodeCheckCast( pVirtualMachineState );
      break;

    case e_JavaOpCodes::NewMultiDimentionalArray:
      ExecuteOpCodeNewMultiDimentionalArray( pVirtualMachineState );
      break;

    case e_JavaOpCodes::LoadReferenceFromConstantPool:
      ExecuteOpCodeLoadReferenceFromConstantPool( pVirtualMachineState );
      break;

    case e_JavaOpCodes::BranchIfEquals:
      ExecutedOpBranchIfEquals( pVirtualMachineState );
      break;

    case e_JavaOpCodes::BranchIfNotEquals:
      ExecutedOpCodeBranchIfNotEquals( pVirtualMachineState );
      break;

    case e_JavaOpCodes::BranchIfLessThan:
      ExecutedOpCodeBranchIfLessThan( pVirtualMachineState );
      break;

    case e_JavaOpCodes::BranchIfGreaterThanEquals:
      ExecutedOpCodeBranchIfGreaterThanOrEqual( pVirtualMachineState );
      break;

    case e_JavaOpCodes::BranchIfGreaterThan:
      ExecutedOpCodeBranchIfGreaterThan( pVirtualMachineState );
      break;

    case e_JavaOpCodes::BranchIfLessThanEquals:
      ExecutedOpCodeBranchIfLessThanOrEqual( pVirtualMachineState );
      break;

    case e_JavaOpCodes::BranchIfIntegerEquals:
      ExecutedOpBranchIfIntegerEquals( pVirtualMachineState );
      break;

    case e_JavaOpCodes::BranchIfIntegerNotEquals:
      ExecutedOpBranchIfIntegerNotEquals( pVirtualMachineState );
      break;

    case e_JavaOpCodes::BranchIfIntegerLessThan:
      ExecutedOpBranchIfIntegerLessThan( pVirtualMachineState );
      break;

    case e_JavaOpCodes::BranchIfIntegerGreaterThanEquals:
      ExecutedOpBranchIfIntegerGreatherThanOrEqual( pVirtualMachineState );
      break;

    case e_JavaOpCodes::BranchIfIntegerGreaterThan:
      ExecutedOpBranchIfIntegerGreatherThan( pVirtualMachineState );
      break;

    case e_JavaOpCodes::BranchIfIntegerLessThanEquals:
      ExecutedOpBranchIfIntegerLessThanOrEqual( pVirtualMachineState );
      break;

    case e_JavaOpCodes::BranchIfReferencesAreEqual:
      ExecuteOpCodeBranchIfReferencesAreEqual( pVirtualMachineState );
      break;

    case e_JavaOpCodes::BranchIfReferencesAreNotEqual:
      ExecuteOpCodeBranchIfReferencesAreNotEqual( pVirtualMachineState );
      break;

    case e_JavaOpCodes::Goto:
      ExecuteOpCodeGoto( pVirtualMachineState );
      break;

    case e_JavaOpCodes::TableSwitch:
      ExecuteOpCodeTableSwitch( pVirtualMachineState );
      break;

    case e_JavaOpCodes::ReturnInteger:
      ExecuteOpCodeReturnInteger( pVirtualMachineState );

      if ( 0 == pVirtualMachineState->GetCallStackDepth() )
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
      ExecuteOpCodeBranchIfNotNull( pVirtualMachineState );
      break;

    case e_JavaOpCodes::BranchIfNull:
      ExecuteOpCodeBranchIfNull( pVirtualMachineState );
      break;

    case e_JavaOpCodes::ConvertIntegerToByte:
      ExecuteOpCodeConvertIntegerToByte( pVirtualMachineState );
      break;

    case e_JavaOpCodes::StoreIntoByteArray:
      ExecuteOpCodeStoreIntoByteArray( pVirtualMachineState );
      break;

    case e_JavaOpCodes::LoadByteOrBooleanFromArray:
      ExecuteOpCodeLoadByteOrBooleanFromArray( pVirtualMachineState );
      break;

    case e_JavaOpCodes::IntegerXOR:
      ExecuteOpCodeXORInteger( pVirtualMachineState );
      break;

    case e_JavaOpCodes::ThrowReference:
      ExecuteOpCodeThrowReference( pVirtualMachineState );
      break;

    case e_JavaOpCodes::LongComparison:
      ExecuteOpCodeLongComparison( pVirtualMachineState );
      break;

    case e_JavaOpCodes::PushDoubleOrLongFromConstantPoolWide:
      ExecuteOpCodePushDoubleOrLongFromConstantPoolWide( pVirtualMachineState );
      break;

    case e_JavaOpCodes::JumpSubRoutine:
      ExecuteOpCodeJumpSubRoutine( pVirtualMachineState );
      break;

    case e_JavaOpCodes::ReturnFromSubRoutine:
      ExecuteOpCodeReturnFromSubRoutine( pVirtualMachineState );
      break;

    case e_JavaOpCodes::ReturnLong:
      ExecuteOpCodeReturnLong( pVirtualMachineState );

      if ( 0 == pVirtualMachineState->GetCallStackDepth() )
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
      ExecuteOpCodeXORLong( pVirtualMachineState );
      break;

    case e_JavaOpCodes::LongAND:
      ExecuteOpCodeANDLong( pVirtualMachineState );
      break;

    case e_JavaOpCodes::LongMultiply:
      ExecuteOpCodeLongMultiply( pVirtualMachineState );
      break;

    case e_JavaOpCodes::LongAdd:
      ExecuteOpCodeLongAdd( pVirtualMachineState );
      break;

    case e_JavaOpCodes::ShiftLongRight:
      ExecuteOpCodeShiftLongRight( pVirtualMachineState );
      break;

    case e_JavaOpCodes::ShiftLongLeft:
      ExecuteOpCodeShiftLongLeft( pVirtualMachineState );
      break;

    case e_JavaOpCodes::ConvertLongToInteger:
      ExecuteOpCodeConvertLongToInteger( pVirtualMachineState );
      break;

    case e_JavaOpCodes::ConvertIntegerToLong:
      ExecuteOpCodeConvertIntegerToLong( pVirtualMachineState );
      break;

    case e_JavaOpCodes::ConvertLongToDouble:
      ExecuteOpCodeConvertLongToDouble( pVirtualMachineState );
      break;

    case e_JavaOpCodes::DoubleDivide:
      ExecuteOpCodeDoubleDivide( pVirtualMachineState );
      break;

    case e_JavaOpCodes::ReturnDouble:
      ExecuteOpCodeReturnDouble( pVirtualMachineState );

      if ( 0 == pVirtualMachineState->GetCallStackDepth() )
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
      ExecuteOpCodeReturnFloat( pVirtualMachineState );

      if ( 0 == pVirtualMachineState->GetCallStackDepth() )
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
      ExecuteOpCodeStoreDoubleInLocalWithIndex( pVirtualMachineState );
      break;

    case e_JavaOpCodes::StoreDoubleInLocal_0:
      ExecuteOpCodeStoreDoubleInLocal( pVirtualMachineState, 0 );
      break;

    case e_JavaOpCodes::StoreDoubleInLocal_1:
      ExecuteOpCodeStoreDoubleInLocal( pVirtualMachineState, 1 );
      break;

    case e_JavaOpCodes::StoreDoubleInLocal_2:
      ExecuteOpCodeStoreDoubleInLocal( pVirtualMachineState, 2 );
      break;

    case e_JavaOpCodes::StoreDoubleInLocal_3:
      ExecuteOpCodeStoreDoubleInLocal( pVirtualMachineState, 3 );
      break;

    case e_JavaOpCodes::LoadDoubleFromLocal:
      ExecuteOpCodeLoadDoubleFromLocalWithIndex( pVirtualMachineState );
      break;

    case e_JavaOpCodes::LoadDoubleFromLocal_0:
      ExecuteOpCodeLoadDoubleFromLocal( pVirtualMachineState, 0 );

      break;

    case e_JavaOpCodes::LoadDoubleFromLocal_1:
      ExecuteOpCodeLoadDoubleFromLocal( pVirtualMachineState, 1 );
      break;

    case e_JavaOpCodes::LoadDoubleFromLocal_2:
      ExecuteOpCodeLoadDoubleFromLocal( pVirtualMachineState, 2 );
      break;

    case e_JavaOpCodes::LoadDoubleFromLocal_3:
      ExecuteOpCodeLoadDoubleFromLocal( pVirtualMachineState, 3 );
      break;

    case e_JavaOpCodes::NoOperation:
      break;

    case e_JavaOpCodes::NegateDouble:
      ExecuteOpCodeNegateDouble( pVirtualMachineState );
      break;

    case e_JavaOpCodes::IntegerAND:
      ExecuteOpCodeANDInteger( pVirtualMachineState );
      break;

    case e_JavaOpCodes::DuplicateTopOperandOrTwo:
      ExecuteOpCodeDuplicateTopOperandOrTwo( pVirtualMachineState );
      break;

    case e_JavaOpCodes::ShiftIntegerRightLogical:
      ExecuteOpCodeShiftIntegerRightLogical( pVirtualMachineState );
      break;

    case e_JavaOpCodes::LoadIntegerFromArray:
      ExecuteOpCodeLoadIntegerFromArray( pVirtualMachineState );
      break;

    case e_JavaOpCodes::IntegerOR:
      ExecuteOpCodeORInteger( pVirtualMachineState );
      break;

    case e_JavaOpCodes::StoreIntoFloatArray:
      ExecuteOpCodeStoreIntoFloatArray( pVirtualMachineState );
      break;

    case e_JavaOpCodes::LoadFloatFromArray:
      ExecuteOpCodeLoadFloatFromArray( pVirtualMachineState );
      break;

    case e_JavaOpCodes::ConvertFloatToDouble:
      ExecuteOpCodeConvertFloatToDouble( pVirtualMachineState );
      break;

    case e_JavaOpCodes::DoubleAdd:
      ExecuteOpCodeDoubleAdd( pVirtualMachineState );
      break;

    case e_JavaOpCodes::DoubleMultiply:
      ExecuteOpCodeDoubleMultiply( pVirtualMachineState );
      break;

    case e_JavaOpCodes::ConvertDoubleToFloat:
      ExecuteOpCodeConvertDoubleToFloat( pVirtualMachineState );
      break;

    case e_JavaOpCodes::FloatDivide:
      ExecuteOpCodeFloatDivide( pVirtualMachineState );
      break;

    case e_JavaOpCodes::ConvertDoubleToInt:
      ExecuteOpCodeConvertDoubleToInt( pVirtualMachineState );
      break;

    case e_JavaOpCodes::StoreFloatInLocal:
      ExecuteOpCodeStoreFloatInLocalWithIndex( pVirtualMachineState );
      break;

    case e_JavaOpCodes::StoreFloatInLocal_0:
      ExecuteOpCodeStoreFloatInLocal( pVirtualMachineState, 0 );
      break;

    case e_JavaOpCodes::StoreFloatInLocal_1:
      ExecuteOpCodeStoreFloatInLocal( pVirtualMachineState, 1 );
      break;

    case e_JavaOpCodes::StoreFloatInLocal_2:
      ExecuteOpCodeStoreFloatInLocal( pVirtualMachineState, 2 );
      break;

    case e_JavaOpCodes::StoreFloatInLocal_3:
      ExecuteOpCodeStoreFloatInLocal( pVirtualMachineState, 3 );
      break;

    case e_JavaOpCodes::FloatSubtract:
      ExecuteOpCodeFloatSubtract( pVirtualMachineState );
      break;

    case e_JavaOpCodes::PushDouble_0:
      ExecuteOpCodePushDouble( pVirtualMachineState, 0.0 );
      break;

    case e_JavaOpCodes::PushDouble_1:
      ExecuteOpCodePushDouble( pVirtualMachineState, 1.0 );
      break;

    case e_JavaOpCodes::DoubleComparisonG:
      ExecuteOpCodeDoubleComparisonG( pVirtualMachineState );
      break;

    case e_JavaOpCodes::DoubleComparisonL:
      ExecuteOpCodeDoubleComparisonL( pVirtualMachineState );
      break;

    case e_JavaOpCodes::FloatAdd:
      ExecuteOpCodeFloatAdd( pVirtualMachineState );
      break;

    case e_JavaOpCodes::DoubleSubtract:
      ExecuteOpCodeDoubleSubtract( pVirtualMachineState );
      break;

    case e_JavaOpCodes::DuplicateTopOperandx2:
      ExecuteOpCodeDuplicateTopOperandx2( pVirtualMachineState );
      break;

    case e_JavaOpCodes::ConvertIntegerToShort:
      ExecuteOpCodeConvertIntegerToShort( pVirtualMachineState );
      break;

    case e_JavaOpCodes::DuplicateTopOperandOrTwox1:
      ExecuteOpCodeDuplicateTopOperandOrTwox1( pVirtualMachineState );
      break;

    case e_JavaOpCodes::LookupSwitch:
      ExecuteOpCodeLookupSwitch( pVirtualMachineState );
      break;

    case e_JavaOpCodes::StoreIntoDoubleArray:
      ExecuteOpCodeStoreIntoDoubleArray( pVirtualMachineState );
      break;

    case e_JavaOpCodes::LoadDoubleFromArray:
      ExecuteOpCodeLoadDoubleFromArray( pVirtualMachineState );
      break;

    case e_JavaOpCodes::LongOr:
      ExecuteOpCodeORLong( pVirtualMachineState );
      break;

    default:
      pLogger->LogDebug( "%s %Id - Read opcode: %hu (%s)\n", pVirtualMachineState->GetCurrentClassAndMethodName().ToUtf8String().c_str(), pVirtualMachineState->GetProgramCounter() - 1, opCode, TranslateOpCode( opCode ) );
      pVirtualMachineState->LogCallStack();
      JVMX_ASSERT( false );
      break;
  }

  return e_ImmediateReturnRequired::No;
}



uint16_t BasicExecutionEngine::GetNextInstruction( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  uint16_t result = pVirtualMachineState->GetCodeSegmentStart()[ pVirtualMachineState->GetProgramCounter() ];
  pVirtualMachineState->AdvanceProgramCounter( 1 );

  return result;
}

void BasicExecutionEngine::ExecuteOpCodeGetStatic( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  // The runtime constant pool item at that index must be a symbolic reference to a field
  std::shared_ptr<ConstantPoolEntry> fieldEntry = pVirtualMachineState->GetConstantFromCurrentClass( ReadIndex( pVirtualMachineState ) );
  if ( e_ConstantPoolEntryTypeFieldReference != fieldEntry->GetType() )
  {
    GetLogger()->LogError( __FUNCTION__ " - Could not resolve field reference for opcode: getstatic" );
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaIncompatibleClassChangeErrorException );
    return;
  }

  const ConstantPoolFieldReference *pFieldRef = dynamic_cast<const ConstantPoolFieldReference *>( fieldEntry->GetValue() );
  if ( nullptr == pFieldRef )
  {
    throw TypeMismatchException( __FUNCTION__ " - Could not convert constant pool entry to field reference." );
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
  catch (FileDoesNotExistException &)
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaJavaClassNotFoundException);
    return;
  }

  JavaString referencedClassName = *pFieldRef->GetType();
  if ( TypeParser::IsReferenceTypeDescriptor( referencedClassName ) && !TypeParser::IsArrayTypeDescriptor( referencedClassName ) )
  {
    referencedClassName = TypeParser::ExtractClassNameFromReference( referencedClassName );
    // On successful resolution of the field, the class or interface that declared the resolved field is initialized if that class or
    // interface has not already been initialized.

    try
    {
      if ( !pVirtualMachineState->IsClassInitialised( referencedClassName ) )
      {
        pVirtualMachineState->InitialiseClass( referencedClassName );
      }
    }
    catch (FileDoesNotExistException&)
    {
      HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaJavaClassNotFoundException);
      return;
    }

  }

  // The value of the class or interface field is fetched and pushed onto the operand stack.
  std::shared_ptr<FieldInfo> pField = ResolveField( pVirtualMachineState, targetClassClassName, *pFieldRef->GetName() );
  if ( nullptr == pField )
  {
    // We would have already thrown a Java Exception
    return;
  }

  //   const CodeAttributeList &attributes = pField->GetAttributes( );
  //   const std::shared_ptr<JavaCodeAttribute> pValue = attributes.at( pField->GetConstantValueIndex( ) );
  //   AttributeConstantValue valueAttribute = pValue->ToConstantValue();

  if ( !pField->IsStatic() )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaIncompatibleClassChangeErrorException );
  }

  //GetLogger()->LogDebug( "Getting Value of field: %s::%s", className.ToByteArray(), *pFieldRef->GetName( )->ToByteArray( ) );

  pVirtualMachineState->PushOperand( pField->GetStaticValue() );
}

ConstantPoolIndex BasicExecutionEngine::ReadIndex( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  if ( !pVirtualMachineState->CanReadBytes( sizeof( ConstantPoolIndex ) ) )
  {
    throw InvalidStateException( __FUNCTION__ " - Not enough space in current code segment to read an index." );
  }

  uint8_t indexByte1 = pVirtualMachineState->GetCodeSegmentStart()[ pVirtualMachineState->GetProgramCounter() ];
  pVirtualMachineState->AdvanceProgramCounter( 1 );
  uint8_t indexByte2 = pVirtualMachineState->GetCodeSegmentStart()[ pVirtualMachineState->GetProgramCounter() ];
  pVirtualMachineState->AdvanceProgramCounter( 1 );

  return ( indexByte1 << 8 ) | indexByte2;
}



std::shared_ptr<FieldInfo> BasicExecutionEngine::ResolveField( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, const JavaString &className, const JavaString &fieldName )
{
  auto it = m_ResolvedFields.find( std::make_pair( className, fieldName ) );
  if ( it != m_ResolvedFields.end() )
  {
    return it->second;
  }

  std::shared_ptr<JavaClass> pClass = ResolveClass( pVirtualMachineState, className );
  if ( nullptr == pClass )
  {
    GetLogger()->LogError( __FUNCTION__ " - Could not resolve Class %s.", className.ToUtf8String().c_str() );

    // We would have already thrown a JavaException.
    return nullptr;
  }

  std::shared_ptr<FieldInfo> pResult = pClass->GetFieldByName( fieldName );
  if ( nullptr == pResult )
  {
    std::shared_ptr<JavaClass> pSuperClass = pClass->GetSuperClass();
    while ( nullptr != pSuperClass )
    {
      if ( nullptr == pSuperClass )
      {
        GetLogger()->LogError( __FUNCTION__ " - Could not resolve Super Class %s.", pClass->GetSuperClass()->GetName()->ToUtf8String().c_str() );

        // We would have already thrown a JavaException.
        return nullptr;
      }

      pResult = pSuperClass->GetFieldByName( fieldName );
      if ( nullptr != pResult )
      {
        break;
      }

      pSuperClass = pSuperClass->GetSuperClass();
    }
  }

  if ( nullptr != pResult )
  {
    m_ResolvedFields.insert( std::make_pair( std::make_pair( className, fieldName ), pResult ) );
  }

  return pResult;
}

std::shared_ptr<JavaClass> BasicExecutionEngine::ResolveClass( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, const JavaString &className )
{
  return HelperClasses::ResolveClass(pVirtualMachineState, className);
}

void BasicExecutionEngine::ExecuteOpCodeLoadReferenceFromLocalWithSpecifiedIndex( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  ExecuteOpCodeLoadReferenceFromLocalIndex( pVirtualMachineState, ReadByteUnsigned( pVirtualMachineState ) );
}

void BasicExecutionEngine::ExecuteOpCodeLoadReferenceFromLocalIndex( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, uint8_t index )
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        pVirtualMachineState->LogLocalVariables();
        pVirtualMachineState->LogOperandStack();
        GetLogger()->LogDebug("Loading reference: %s from local variable %d", pVirtualMachineState->GetLocalVariable(index)->ToString().ToUtf8String().c_str(), static_cast<int>(index));
    }
#endif // _DEBUG

  pVirtualMachineState->PushOperand( pVirtualMachineState->GetLocalVariable( index ) );
}

void BasicExecutionEngine::ExecuteOpCodeReturnVoid( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#ifdef _DEBUG
  const size_t debugSize = pVirtualMachineState->GetOperandStackSize();
#endif // _DEBUG

  if ( pVirtualMachineState->GetCurrentMethodInfo()->IsSynchronised() )
  {
    pVirtualMachineState->PopMonitor()->Unlock( pVirtualMachineState->GetCurrentMethodInfo()->GetFullName().ToUtf8String().c_str() );
  }

  pVirtualMachineState->PopState();
  pVirtualMachineState->ReleaseLocalVariables();

#ifdef _DEBUG
  JVMX_ASSERT( pVirtualMachineState->GetOperandStackSize() == debugSize );
#endif // _DEBUG
}

e_IncreaseCallStackDepth BasicExecutionEngine::ExecuteOpCodeInvokeStatic( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  // The unsigned indexbyte1 and indexbyte2 are used to construct an index into the runtime constant pool of the current class

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        pVirtualMachineState->LogOperandStack();
        pVirtualMachineState->LogCallStack();
    }
#endif // _DEBUG

  boost::intrusive_ptr<JavaString> pClassName = new JavaString( JavaString::EmptyString() );
  std::shared_ptr<MethodInfo> pMethodInfo = ResolveMethodReference( pVirtualMachineState, ReadIndex( pVirtualMachineState ), *pClassName );
  if ( !pMethodInfo->IsStatic() || pMethodInfo->IsAbstract() )
  {
    pVirtualMachineState->LogLocalVariables();

#ifdef _DEBUG
    GetLogger()->LogDebug( __FUNCTION__ " - Throwing exception because method is not static or is abstract: %s", pMethodInfo->GetName()->ToUtf8String().c_str() );
#endif // _DEBUG

    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaIncompatibleClassChangeErrorException );
    return e_IncreaseCallStackDepth::No;
  }

  // On successful resolution of the field, the class or interface that declared the resolved field is initialized if that class or
  // interface has not already been initialized.
  if ( !pVirtualMachineState->IsClassInitialised( *pClassName ) )
  {
    pVirtualMachineState->InitialiseClass( *pClassName );
  }

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      GetLogger()->LogDebug("Method type: %s.", pMethodInfo->GetType()->ToUtf8String().c_str());
  }
#endif // _DEBUG

  // TODO: Pop nargs from operand stack where the   number, type, and order of the values must be consistent with the descriptor of the resolved method

  if ( pMethodInfo->IsSynchronised() )
  {
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
      if (pVirtualMachineState->HasUserCodeStarted())
      {
          GetLogger()->LogDebug("Static Method %s is synchronized.", pMethodInfo->GetName()->ToUtf8String().c_str());
      }
#endif // _DEBUG

    pVirtualMachineState->PushMonitor( pMethodInfo->GetClass()->MonitorEnter( pMethodInfo->GetFullName().ToUtf8String().c_str() ) );
  }

  if ( pMethodInfo->IsNative() )
  {
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
      if (pVirtualMachineState->HasUserCodeStarted())
      {
          GetLogger()->LogDebug("Method implementation is native: %s type: %s.", pMethodInfo->GetName()->ToUtf8String().c_str(), pMethodInfo->GetType()->ToUtf8String().c_str());
      }
#endif // _DEBUG

    TypeParser::ParsedMethodType type = TypeParser::ParseMethodType( *( pMethodInfo->GetType() ) );
    boost::intrusive_ptr<ObjectReference> pJavaLangClass = pVirtualMachineState->CreateJavaLangClassFromClassName( pClassName );

    std::shared_ptr<JavaNativeInterface> pJNI = pVirtualMachineState->GetJavaNativeInterface();

    //pVirtualMachineState->PushState( *( pMethodInfo->GetClass()->GetName() ), *( pMethodInfo->GetName() ), *( pMethodInfo->GetType() ), pMethodInfo );
    pJNI->ExecuteFunctionStatic( JavaNativeInterface::MakeJNINameWithoutArgumentDescriptor( pClassName, pMethodInfo->GetName() ), pMethodInfo->GetType(), pJavaLangClass );
    //pVirtualMachineState->PopState();

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        pVirtualMachineState->LogOperandStack();
    }
#endif

    if ( pMethodInfo->IsSynchronised() )
    {
      pVirtualMachineState->PopMonitor();
      pMethodInfo->GetClass()->MonitorExit( pMethodInfo->GetFullName().ToUtf8String().c_str() );
    }

    return e_IncreaseCallStackDepth::No;
  }
  else
  {
    // values are consecutively made the values of local variables of the new frame, with arg1 in local variable 0 (or, if arg1 is of type long or double, in local variables 0 and 1) and so on
    //pVirtualMachineState->PushState( *pClassName, *(pMethodInfo->GetName()), *(pMethodInfo->GetType()) );
    pVirtualMachineState->PushState( *( pMethodInfo->GetClass()->GetName() ), *( pMethodInfo->GetName() ), *( pMethodInfo->GetType() ), pMethodInfo );
    pVirtualMachineState->SetCodeSegment( pMethodInfo->GetCodeInfo() );
    pVirtualMachineState->SetupLocalVariables( pMethodInfo );
  }

  return e_IncreaseCallStackDepth::Yes;
}

std::shared_ptr<MethodInfo> BasicExecutionEngine::ResolveMethodReference( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, uint16_t methodIndex, JavaString &className )
{
  // The runtime constant pool item at that index must be a symbolic reference to a field
  std::shared_ptr<ConstantPoolEntry> methodEntry = pVirtualMachineState->GetConstantFromCurrentClass( methodIndex );
  if ( e_ConstantPoolEntryTypeMethodReference != methodEntry->GetType() )
  {
    GetLogger()->LogError( __FUNCTION__ " - Could not resolve method reference: %hu", methodIndex );
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaIncompatibleClassChangeErrorException );
    return nullptr;
  }

  const ConstantPoolMethodReference *pMethodRef = dynamic_cast<const ConstantPoolMethodReference *>( methodEntry->GetValue() );
  if ( nullptr == pMethodRef )
  {
    throw TypeMismatchException( __FUNCTION__ " - Could not convert constant pool entry to field reference." );
  }

  // The referenced field is resolved
  boost::intrusive_ptr<JavaString> pClassName = pMethodRef->GetClassName();
  className = *pClassName;

  std::shared_ptr<MethodInfo> pMethodInfo = pVirtualMachineState->ResolveMethodOnClass( pClassName, pMethodRef );

  return pMethodInfo;
}

void BasicExecutionEngine::ExecuteOpCodeMonitorEnter( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  auto pOperand = pVirtualMachineState->PopOperand();

  if ( e_JavaVariableTypes::Object == pOperand->GetVariableType() )
  {
    boost::intrusive_ptr<ObjectReference> pReference = boost::dynamic_pointer_cast<ObjectReference>( pOperand );
    if ( nullptr == pReference )
    {
      throw InvalidStateException( __FUNCTION__ " - Expected object on operand stack." );
    }

#ifdef _DEBUG
    if ( pReference->GetContainedObject()->GetClass()->GetName()->Contains( u"VMThread" ) )
    {
      //__asm int 3;
    }
#endif // _DEBUG

    pReference->GetContainedObject()->MonitorEnter( pVirtualMachineState->GetCurrentMethodInfo()->GetFullName().ToUtf8String().c_str() );
  }
  else if ( e_JavaVariableTypes::Array == pOperand->GetVariableType() )
  {
    boost::intrusive_ptr<ObjectReference> pReference = boost::dynamic_pointer_cast<ObjectReference>( pOperand );
    if ( nullptr == pReference )
    {
      throw InvalidStateException( __FUNCTION__ " - Expected array on operand stack." );
    }

    pReference->GetContainedArray()->MonitorEnter( pVirtualMachineState->GetCurrentClassAndMethodName().ToUtf8String().c_str() );
  }
  else if ( e_JavaVariableTypes::ClassReference == pOperand->GetVariableType() )
  {
    boost::intrusive_ptr<JavaClassReference> pReference = boost::dynamic_pointer_cast<JavaClassReference>( pOperand );
    if ( nullptr == pReference )
    {
      throw InvalidStateException( __FUNCTION__ " - Expected class on operand stack." );
    }

    pReference->GetClassFile()->MonitorEnter( pVirtualMachineState->GetCurrentClassAndMethodName().ToUtf8String().c_str() );
  }
  else
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected object, class or array on operand stack." );
  }
}

void BasicExecutionEngine::ExecuteOpCodePushNull( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  pVirtualMachineState->PushOperand( new ObjectReference( nullptr ) );
}

void BasicExecutionEngine::ExecuteOpCodeNewMultiDimentionalArray( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  uint16_t index = ReadIndex( pVirtualMachineState );

  uint8_t dimentionCount = ReadByteUnsigned( pVirtualMachineState );
  JVMX_ASSERT( dimentionCount >= 1 );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      pVirtualMachineState->LogOperandStack();
  }
#endif

#ifdef _DEBUG
  if ( dimentionCount > 2 )
  {
    // We have not tested with more than 2 dimentions yet.
    BreakDebug( "a", "a" );
  }
#endif // _DEBUG

  std::vector<int32_t> dimentionSizes;
  for ( uint8_t i = 0; i < dimentionCount; ++i )
  {
    JVMX_ASSERT( e_JavaVariableTypes::Integer == pVirtualMachineState->PeekOperand()->GetVariableType() );
    dimentionSizes.push_back( boost::dynamic_pointer_cast<JavaInteger>( pVirtualMachineState->PopOperand() )->ToHostInt32() );
  }

  std::shared_ptr<ConstantPoolEntry> pConstantPoolEntry = pVirtualMachineState->GetConstantFromCurrentClass( index );
  std::shared_ptr<ConstantPoolClassReference> pClassRef = pConstantPoolEntry->AsClassReferencePointer();

  e_JavaArrayTypes finalDimentionType = TypeParser::ExtractContainedTypeFromArrayTypeDescriptor( *pClassRef->GetClassName() );

  if ( 1 == dimentionCount ) // Just handle this edge case.
  {
    boost::intrusive_ptr<ObjectReference> pFirstDimention = pVirtualMachineState->CreateArray( finalDimentionType, dimentionSizes[ 0 ] );
    pVirtualMachineState->PushOperand( pFirstDimention );
    return;
  }

  boost::intrusive_ptr<ObjectReference> pFirstDimention = pVirtualMachineState->CreateArray( e_JavaArrayTypes::Reference, dimentionSizes[ 0 ] );

  int32_t currentDimention = 0;
  InitialiseDimention( dimentionSizes, pVirtualMachineState, pFirstDimention, dimentionCount, currentDimention, finalDimentionType );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      GetLogger()->LogDebug("New Multi Array: %s", pFirstDimention->ToString().ToUtf8String().c_str());
  }
#endif

  pVirtualMachineState->PushOperand( pFirstDimention );
}

void BasicExecutionEngine::InitialiseDimention( const std::vector<int32_t> &dimentionSizes, const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, boost::intrusive_ptr<ObjectReference> pFirstDimention, uint8_t dimentionCount, int32_t currentDimention, e_JavaArrayTypes finalDimentionType )
{
  e_JavaArrayTypes arrayTypeForCurrentDimention = e_JavaArrayTypes::Reference;
  int32_t currentDimentionSize = dimentionSizes[ currentDimention ];

  if ( currentDimention == dimentionCount - 2 )
  {
    arrayTypeForCurrentDimention = finalDimentionType;
  }

  for ( int32_t j = 0; j < dimentionSizes[ currentDimention ]; ++j )
  {
    boost::intrusive_ptr<ObjectReference> pNextDim = pVirtualMachineState->CreateArray( arrayTypeForCurrentDimention, currentDimentionSize );
    pFirstDimention->GetContainedArray()->SetAt( j, pNextDim.get() );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        GetLogger()->LogDebug("New inner Array: %s", pNextDim->ToString().ToUtf8String().c_str());
    }
#endif

    if ( currentDimention < dimentionCount - 2 )
    {
      InitialiseDimention( dimentionSizes, pVirtualMachineState, pNextDim, dimentionCount, currentDimention + 1, finalDimentionType );
    }
  }
}

void BasicExecutionEngine::ExecuteOpCodeLoadDoubleFromArray( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  if ( e_JavaVariableTypes::Integer != pVirtualMachineState->PeekOperand()->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected integer on the operand stack." );
  }

  boost::intrusive_ptr< JavaInteger > pIndex = boost::dynamic_pointer_cast<JavaInteger>( pVirtualMachineState->PopOperand() );

  if ( e_JavaVariableTypes::Array != pVirtualMachineState->PeekOperand()->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected Array on the operand stack." );
  }

  boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pArray )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected reference to JavaArray on operand stack." );
  }

  if ( pArray->IsNull() )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaNullPointerExceptionException );
    return;
  }

  if ( pIndex->ToHostInt32() < 0 || pIndex->ToHostInt32() > static_cast<int32_t>( pArray->GetContainedArray()->GetNumberOfElements() ) )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaArrayIndexOutOfBoundsException );
    return;
  }

  JavaDouble *pTempDouble = dynamic_cast<JavaDouble *>( pArray->GetContainedArray()->At( pIndex->ToHostInt32() ) );
  if ( nullptr == pTempDouble )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected double in array." );
  }

  boost::intrusive_ptr<JavaDouble> pDouble = new JavaDouble( JavaDouble::FromHostDouble( pTempDouble->ToHostDouble() ) );
  pVirtualMachineState->PushOperand( pDouble );
}

void BasicExecutionEngine::ExecuteOpCodeORLong( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        pVirtualMachineState->LogOperandStack();
    }
#endif // _DEBUG

  int64_t long2 = GetLongFromOperandStack( pVirtualMachineState );
  int64_t long1 = GetLongFromOperandStack( pVirtualMachineState );

  pVirtualMachineState->PushOperand( new JavaLong( JavaLong::FromHostInt64( long1 | long2 ) ) );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeLookupSwitch( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  intptr_t startingAddress = pVirtualMachineState->GetProgramCounter() - 1; // -1 for the size of the switch instruction.

  intptr_t paddingLength = 4 - ( pVirtualMachineState->GetProgramCounter() % 4 );

  JVMX_ASSERT( paddingLength < 4 && paddingLength >= 0 );

  for ( int i = 0; i < paddingLength; ++ i )
  {
    uint8_t readByte = ReadByteUnsigned( pVirtualMachineState );
    readByte += 0;
  }

  JVMX_ASSERT( ( pVirtualMachineState->GetProgramCounter() % 4 ) == 0 );

  const int32_t deflt = Read32BitOffset( pVirtualMachineState );
  const int32_t npairs = Read32BitOffset( pVirtualMachineState );

  JVMX_ASSERT( npairs >= 0 );

  std::vector<std::pair<int32_t, int32_t>> jumpTable;
  for ( int i = 0; i < npairs; ++ i )
  {
    int32_t match = Read32BitOffset( pVirtualMachineState );
    int32_t offset = Read32BitOffset( pVirtualMachineState );

    jumpTable.push_back( std::make_pair( match, offset ) );
  }

  boost::intrusive_ptr< JavaInteger > pKey = boost::dynamic_pointer_cast<JavaInteger>( pVirtualMachineState->PopOperand() );

  intptr_t targetAddress = 0;
  for ( int i = 0; i < npairs; ++ i )
  {
    if ( jumpTable[i].first == pKey->ToHostInt32() )
    {
      targetAddress = startingAddress + jumpTable[ i ].second;
      break;
    }
  }

  if ( 0 == targetAddress )
  {
    targetAddress = startingAddress + deflt;
  }

  pVirtualMachineState->AdvanceProgramCounter( targetAddress - pVirtualMachineState->GetProgramCounter() );
}

void BasicExecutionEngine::ExecuteOpCodeStoreIntoDoubleArray( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#if defined _DEBUG && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        pVirtualMachineState->LogCallStack();
        pVirtualMachineState->LogOperandStack();
    }
#endif // _DEBUG

  boost::intrusive_ptr<IJavaVariableType> pValue = pVirtualMachineState->PopOperand();
  if ( e_JavaVariableTypes::Double != pValue->GetVariableType() )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected third operand to be double type." );
  }

  boost::intrusive_ptr< JavaInteger > pIndex = boost::dynamic_pointer_cast<JavaInteger>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pIndex )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected second operand to be integer type." );
  }

  boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pArray )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected first operand to be array type." );
  }

  if ( pArray->IsNull() )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaNullPointerExceptionException );
  }

  if ( pIndex->ToHostInt32() < 0 || static_cast<size_t>( pIndex->ToHostInt32() ) > pArray->GetContainedArray()->GetNumberOfElements() )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaArrayIndexOutOfBoundsException );
  }

  pArray->GetContainedArray()->SetAt( *pIndex, pValue.get() );
}

void BasicExecutionEngine::ExecuteOpCodeDuplicateTopOperandOrTwox1( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
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

  if ( IsCategoryOneType( pValue1 ) && IsCategoryOneType( pValue2 ) )
  {
    auto pValue3 = pVirtualMachineState->PopOperand();
    JVMX_ASSERT( IsCategoryOneType( pValue3 ) && IsCategoryOneType( pValue2 ) );

    pVirtualMachineState->PushOperand( pValue2 );
    pVirtualMachineState->PushOperand( pValue1 );

    pVirtualMachineState->PushOperand( pValue3 );
    pVirtualMachineState->PushOperand( pValue2 );
    pVirtualMachineState->PushOperand( pValue1 );
  }
  else
  {
    JVMX_ASSERT( IsCategoryTwoType( pValue1 ) && IsCategoryOneType( pValue2 ) );

    pVirtualMachineState->PushOperand( pValue1 );
    pVirtualMachineState->PushOperand( pValue2 );
    pVirtualMachineState->PushOperand( pValue1 );
  }

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      GetLogger()->LogDebug("**After:");
      pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeConvertIntegerToShort( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  boost::intrusive_ptr< JavaInteger > pInteger1 = boost::dynamic_pointer_cast<JavaInteger>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pInteger1 )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected first operand to be integer type." );
  }

  boost::intrusive_ptr<JavaShort> pFinalShort = new JavaShort( JavaShort::FromHostInt16( static_cast<uint16_t>( pInteger1->ToHostInt32() ) ) );
  pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromShort( *pFinalShort ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeFloatAdd( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        pVirtualMachineState->LogOperandStack();
    }
#endif // _DEBUG
  boost::intrusive_ptr<IJavaVariableType> pOperand2 = pVirtualMachineState->PopOperand();
  boost::intrusive_ptr<IJavaVariableType> pOperand1 = pVirtualMachineState->PopOperand();

  if ( e_JavaVariableTypes::Float != pOperand2->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected a float on the operand stack." );
  }

  if ( e_JavaVariableTypes::Float != pOperand1->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected a float on the operand stack." );
  }

  float value2 = boost::dynamic_pointer_cast<JavaFloat>( pOperand2 )->ToHostFloat();
  float value1 = boost::dynamic_pointer_cast<JavaFloat>( pOperand1 )->ToHostFloat();

  pVirtualMachineState->PushOperand( new JavaFloat( JavaFloat::FromHostFloat( value1 + value2 ) ) );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodePushDouble( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, double value )
{
  pVirtualMachineState->PushOperand( new JavaDouble( JavaDouble::FromHostDouble( value ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeFloatSubtract( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  float float2 = boost::dynamic_pointer_cast<JavaFloat>( pVirtualMachineState->PopOperand() )->ToHostFloat();
  float float1 = boost::dynamic_pointer_cast<JavaFloat>( pVirtualMachineState->PopOperand() )->ToHostFloat();

  pVirtualMachineState->PushOperand( new JavaFloat( JavaFloat::FromHostFloat( float1 - float2 ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeStoreFloatInLocal( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, uint8_t localVariableIndex )
{
  if ( e_JavaVariableTypes::Float != pVirtualMachineState->PeekOperand()->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected float on the operand stack." );
  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      GetLogger()->LogDebug("Storing float %s in local variable %d", pVirtualMachineState->PeekOperand()->ToString().ToUtf8String().c_str(), localVariableIndex);
  }
#endif // _DEBUG

  pVirtualMachineState->SetLocalVariable( localVariableIndex, pVirtualMachineState->PopOperand() );
}

void BasicExecutionEngine::ExecuteOpCodeStoreFloatInLocalWithIndex( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  ExecuteOpCodeStoreFloatInLocal( pVirtualMachineState, ReadByteUnsigned( pVirtualMachineState ) );
}

void BasicExecutionEngine::ExecuteOpCodePutStatic( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        pVirtualMachineState->LogOperandStack();
        pVirtualMachineState->LogCallStack();
    }
#endif // _DEBUG

  // The runtime constant pool item at that index must be a symbolic reference to a field
  std::shared_ptr<ConstantPoolFieldReference> pFieldEntry = pVirtualMachineState->GetConstantFromCurrentClass( ReadIndex( pVirtualMachineState ) )->AsFieldReferencePointer();

  //std::shared_ptr<FieldInfo> pFieldInfo = ResolveField( pVirtualMachineState->GetCurrentClassName(), *pFieldEntry->GetName() );
  std::shared_ptr<FieldInfo> pFieldInfo = ResolveField( pVirtualMachineState, *pFieldEntry->GetClassName(), *pFieldEntry->GetName() );
  if ( nullptr == pFieldInfo )
  {
    GetLogger()->LogError( __FUNCTION__ " - Could not resolve field reference for opcode: getstatic" );
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaIncompatibleClassChangeErrorException );
    return;
  }

  if ( !pVirtualMachineState->IsClassInitialised( *pFieldEntry->GetClassName() ) )
  {
    pVirtualMachineState->InitialiseClass( *pFieldEntry->GetClassName() );
  }

  if ( !pFieldInfo->IsStatic() )
  {
    GetLogger()->LogError( __FUNCTION__ " - Throwing Java Exception because field is not static." );
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaIncompatibleClassChangeErrorException );
  }

  if ( pFieldInfo->IsFinal() && !( pVirtualMachineState->GetCurrentMethodName() == c_ClassInitialisationMethodName ) )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaIllegalAccessErrorException );
  }

  boost::intrusive_ptr<IJavaVariableType> pOperand = pVirtualMachineState->PopOperand();

  // Now Assign the Value

#ifdef _DEBUG
  ValidateStaticFieldType( pFieldEntry, pFieldInfo, pOperand );
#endif // _DEBUG

  pFieldInfo->SetStaticValue( pOperand );
}

const char *BasicExecutionEngine::TranslateOpCode( uint16_t opcode )
{
  switch ( (e_JavaOpCodes)opcode )
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

    case e_JavaOpCodes::StoreIntoCharArray:
      return "castore";
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

    case e_JavaOpCodes::IntegerRemainder:
      return "irem";
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

void BasicExecutionEngine::ExecuteOpCodeLoadReferenceFromConstantPool( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  uint8_t indexByte = pVirtualMachineState->GetCodeSegmentStart()[ pVirtualMachineState->GetProgramCounter() ];
  pVirtualMachineState->AdvanceProgramCounter( 1 );

  std::shared_ptr<ConstantPoolEntry> pConstant = pVirtualMachineState->GetConstantFromCurrentClass( indexByte );

  if ( e_ConstantPoolEntryTypeInteger == pConstant->GetType() )
  {
    pVirtualMachineState->PushOperand( pConstant->AsInteger() );
  }
  else if ( e_ConstantPoolEntryTypeFloat == pConstant->GetType() )
  {
    pVirtualMachineState->PushOperand( pConstant->AsFloat() );
  }
  else if ( e_ConstantPoolEntryTypeString == pConstant->GetType() )
  {
    pVirtualMachineState->PushOperand( pConstant->AsString() );
  }
  else if ( e_ConstantPoolEntryTypeStringReference == pConstant->GetType() )
  {
    std::shared_ptr<ConstantPoolStringReference> pStringRefRef = pConstant->AsStringReference();

    boost::intrusive_ptr<ObjectReference> pObject = GetFromStringPool(pStringRefRef);
    if (nullptr == pObject)
    {
      pObject = pVirtualMachineState->CreateStringObject(*pStringRefRef->GetStringValue());
      AddToStringPool(pStringRefRef, pObject);
    }

    pVirtualMachineState->PushOperand( pObject );
  }
  else if ( e_ConstantPoolEntryTypeClassReference == pConstant->GetType() )
  {
    std::shared_ptr<ConstantPoolClassReference> pClassRef = pConstant->AsClassReferencePointer();
    std::shared_ptr<JavaClass> pClassFile = ResolveClass( pVirtualMachineState, *pClassRef->GetClassName() );

    if ( nullptr == pClassFile )
    {
      throw InvalidStateException( __FUNCTION__ " - Class was not resolved." );
    }

    pVirtualMachineState->PushOperand( new JavaClassReference( pClassFile ) );
  }
  else if ( e_ConstantPoolEntryTypeMethodHandle == pConstant->GetType() || e_ConstantPoolEntryTypeMethodType == pConstant->GetType() )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }
  else if ( e_ConstantPoolEntryTypeMethodReference == pConstant->GetType() )
  {
    //     std::shared_ptr<JavaClassFile> pClass = GetClassLibrary()->FindClass( *(pConstant->AsMethodReference()->GetName()) );
    //     pVirtualMachineState->PushOperand( pClass->GetMethodByNameAndType( *(pConstant->AsMethodReference( )->GetName( )), *(pConstant->AsMethodReference( )->GetType( )) ) );
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }
  else
  {
    throw InvalidArgumentException( __FUNCTION__ " - Unexpected constant type." );
  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      GetLogger()->LogDebug("Constant Value: %s", pVirtualMachineState->PeekOperand()->ToString().ToUtf8String().c_str());
  }
#endif // _DEBUG
}

e_IncreaseCallStackDepth BasicExecutionEngine::ExecuteOpCodeInvokeVirtual( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  std::shared_ptr<ConstantPoolEntry> pConstant = pVirtualMachineState->GetConstantFromCurrentClass( ReadIndex( pVirtualMachineState ) );
  if ( pConstant->GetType() != e_ConstantPoolEntryTypeMethodReference )
  {
    pVirtualMachineState->LogCallStack();
    throw InvalidStateException( __FUNCTION__ " - Expected Method reference." );
  }

  return ExecuteVirtualMethod( pVirtualMachineState, pConstant );
}

void BasicExecutionEngine::ExecuteOpCodePushInt( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, int value )
{
  pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( value ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeNewArrayOfReference( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  ConstantPoolIndex index = ReadIndex( pVirtualMachineState );

  boost::intrusive_ptr< JavaInteger > pCount = boost::dynamic_pointer_cast<JavaInteger>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pCount )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected an operand of type int." );
  }

  if ( pCount->ToHostInt32() < 0 )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaNegativeArraySizeExceptionException );
    return;
  }

  //std::shared_ptr<JavaClass> pClass = ResolveClassFromIndex( index );

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      GetLogger()->LogDebug("Creating array of type reference");
  }
#endif

  boost::intrusive_ptr<ObjectReference> pArray = pVirtualMachineState->CreateArray( e_JavaArrayTypes::Reference, pCount->ToHostInt32() );

  pVirtualMachineState->PushOperand( pArray );

  //pVirtualMachineState->PushOperand( new JavaArray( e_JavaArrayTypes::Reference, pCount->ToHostInt32() ) );
}

void BasicExecutionEngine::ExecuteOpCodeNew( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  std::shared_ptr<JavaClass> pClass = ResolveClassFromIndex( pVirtualMachineState, ReadIndex( pVirtualMachineState ) );

  if ( nullptr == pClass )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaInstantiationErrorException );
    throw InvalidStateException( __FUNCTION__ " - Could not resolve class from index." );
  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      GetLogger()->LogDebug("Creating object of type: %s", pClass->GetName()->ToUtf8String().c_str());
  }
#endif // defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)

  if ( !pClass->IsInitialsed() )
  {
    pVirtualMachineState->InitialiseClass( *pClass->GetName() );
  }

  pVirtualMachineState->PushOperand( pVirtualMachineState->CreateObject( pClass ) );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      pVirtualMachineState->LogOperandStack();
  }
#endif // defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
}

std::shared_ptr<JavaClass> BasicExecutionEngine::ResolveClassFromIndex( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, ConstantPoolIndex index )
{
  std::shared_ptr<ConstantPoolClassReference> pClassRef = pVirtualMachineState->GetConstantFromCurrentClass( index )->AsClassReferencePointer();
  if ( nullptr == pClassRef )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected index to be a reference to a class/interface/array." );
  }

  return ResolveClass( pVirtualMachineState, *pClassRef->GetClassName() );
}

void BasicExecutionEngine::ExecuteOpCodeDuplicateTopOperand( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        GetLogger()->LogDebug("Inside dup...");
        pVirtualMachineState->LogOperandStack();
    }
#endif // _DEBUG

  auto pTop = pVirtualMachineState->PeekOperand();
  pVirtualMachineState->PushOperand( pTop );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeDuplicateTopOperandx1( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        pVirtualMachineState->LogOperandStack();
    }
#endif // _DEBUG

  auto pValue1 = pVirtualMachineState->PopOperand();
  auto pValue2 = pVirtualMachineState->PopOperand();

  pVirtualMachineState->PushOperand( pValue1 );
  pVirtualMachineState->PushOperand( pValue2 );
  pVirtualMachineState->PushOperand( pValue1 );

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

e_IncreaseCallStackDepth BasicExecutionEngine::ExecuteOpCodeInvokeSpecial( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  ConstantPoolIndex index = ReadIndex( pVirtualMachineState );

  std::shared_ptr<ConstantPoolMethodReference> pMethodRef = pVirtualMachineState->GetConstantFromCurrentClass( index )->AsMethodReference();
  std::shared_ptr<MethodInfo> pMethodInfo = pVirtualMachineState->ResolveMethodOnClass( pMethodRef->GetClassName(), pMethodRef.get() );

  if ( nullptr == pMethodInfo )
  {
    throw InvalidStateException( __FUNCTION__ " - Could not resolve method." );
  }

  // Finally, if the resolved method is protected, and it is a member of a superclass of the current class, and the method is not
  // declared in the same runtime package as the current class, then the class of *objectref* must be either the current class or a subclass of the current class.
  if ( pMethodInfo->IsProtected() )
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
  std::shared_ptr<JavaClass> pFinalClass( GetClassLibrary()->FindClass( *pMethodRef->GetClassName() ) );

  if ( pVirtualMachineState->GetCurrentClass()->IsSuper() && IsSuperClassOfCurrentClass( pVirtualMachineState, pMethodRef->GetClassName() ) && *( pMethodRef->GetName() ) != c_InstanceInitialisationMethodName )
  {
    // If the above conditions are true, the actual method to be invoked let C be the direct superclass of the current class
    std::shared_ptr<JavaClass> pSuperClass = pVirtualMachineState->GetCurrentClass()->GetSuperClass();
    if ( nullptr != pSuperClass )
    {
      // If C contains a declaration for an instance method with the same name and descriptor as the resolved method, then this method will be invoked.
      pFinalMethod = pSuperClass->GetMethodByNameAndType( *( pMethodRef->GetName() ), *pMethodRef->GetType() );
      pFinalClass = pSuperClass;
      if ( nullptr == pFinalMethod )
      {
        // Otherwise, if C has a superclass, this same lookup procedure is performed recursively using the direct superclass of C.The
        // method to be invoked is the result of the recursive invocation   of this lookup procedure.
        pSuperClass = pSuperClass->GetSuperClass();
        while ( nullptr  != pSuperClass )
        {
          pFinalMethod = pSuperClass->GetMethodByNameAndType( *( pMethodRef->GetName() ), *pMethodRef->GetType() );

          if ( nullptr != pFinalMethod )
          {
            pFinalClass = pSuperClass;
            break;
          }

          pSuperClass = pSuperClass->GetSuperClass();
        }

        if ( nullptr == pFinalMethod )
        {
          HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaAbstractMethodErrorException );
          return e_IncreaseCallStackDepth::No;
        }
      }
    }
  }

  boost::intrusive_ptr<JavaString> pFinalMethodName = pFinalMethod->GetName();
  boost::intrusive_ptr<JavaString> pFinalMethodType = pFinalMethod->GetType();
  boost::intrusive_ptr<JavaString> pFinalClassName = pFinalClass->GetName();

  if ( !pFinalMethod->IsNative() )
  {
    //If the method is not native, the nargs argument values and objectref are popped from the operand stack.

    pVirtualMachineState->PushState( *pFinalClassName, *pFinalMethodName, *pFinalMethodType, pFinalMethod );
    boost::intrusive_ptr<ObjectReference> pObject = pVirtualMachineState->SetupLocalVariables( pFinalMethod );

    if ( pFinalMethod->IsSynchronised() )
    {
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
        if (pVirtualMachineState->HasUserCodeStarted())
        {
            GetLogger()->LogDebug("Special Method is synchronized _ 3: %s.", pFinalMethodType->ToUtf8String().c_str());
        }
#endif // _DEBUG
      pVirtualMachineState->PushMonitor( pObject->GetContainedObject()->MonitorEnter( pFinalMethod->GetFullName().ToUtf8String().c_str() ) );
    }

    pVirtualMachineState->UpdateCurrentClassName( pFinalClassName );
    pVirtualMachineState->SetCodeSegment( pFinalMethod->GetCodeInfo() );

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
  TypeParser::ParsedMethodType parsedType = TypeParser::ParseMethodType( *pFinalMethodType );
  uint8_t objectIndex = static_cast<uint8_t>( parsedType.parameters.size() + 1 );

  boost::intrusive_ptr<ObjectReference> pObject = boost::dynamic_pointer_cast<ObjectReference>( pVirtualMachineState->PeekOperandFromBack( objectIndex ) );

  if ( nullptr == pObject )
  {
    throw InvalidStateException( __FUNCTION__ " - Could not find object on the operand stack." );
  }

  if ( pFinalMethod->IsStatic() )
  {
    if ( pFinalMethod->IsSynchronised() )
    {
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
        if (pVirtualMachineState->HasUserCodeStarted())
        {
            GetLogger()->LogDebug("Special Method is synchronized _ 1: %s.", pFinalMethodType->ToUtf8String().c_str());
        }
#endif // _DEBUG
      pVirtualMachineState->PushMonitor( pFinalClass->MonitorEnter( pFinalMethod->GetFullName().ToUtf8String().c_str() ) );
    }

    std::shared_ptr<JavaNativeInterface> pJNI = pVirtualMachineState->GetJavaNativeInterface();

    //pVirtualMachineState->PushState( *( pFinalMethod->GetClass()->GetName() ), *( pFinalMethod->GetName() ), *( pFinalMethod->GetType() ), pFinalMethod );
    //pVirtualMachineState->SetupLocalVariables( pFinalMethod );

    pJNI->ExecuteFunctionStatic( JavaNativeInterface::MakeJNINameWithoutArgumentDescriptor( pFinalClassName, pFinalMethodName ), pFinalMethodType, pObject );

    //pVirtualMachineState->PopState();

    if ( pFinalMethod->IsSynchronised() )
    {
      pVirtualMachineState->PopMonitor()->Unlock( pFinalMethod->GetFullName().ToUtf8String().c_str() );
    }
  }
  else
  {
    if ( pFinalMethod->IsSynchronised() )
    {
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
        if (pVirtualMachineState->HasUserCodeStarted())
        {
            GetLogger()->LogDebug("Method is synchronized _ 2 : %s.", pFinalMethodType->ToUtf8String().c_str());
        }
#endif // _DEBUG
      pVirtualMachineState->PushMonitor( pObject->GetContainedObject()->MonitorEnter( pFinalMethod->GetFullName().ToUtf8String().c_str() ) );
    }

    std::shared_ptr<JavaNativeInterface> pJNI = pVirtualMachineState->GetJavaNativeInterface();

    //pVirtualMachineState->PushState( *( pMethodInfo->GetClass()->GetName() ), *( pMethodInfo->GetName() ), *( pMethodInfo->GetType() ), pMethodInfo );
    //pVirtualMachineState->SetupLocalVariables( pFinalMethod );

    pJNI->ExecuteFunction( JavaNativeInterface::MakeJNINameWithoutArgumentDescriptor( pFinalClassName, pFinalMethodName ), pFinalMethodType, pObject );

    //pVirtualMachineState->PopState();

    if ( pFinalMethod->IsSynchronised() )
    {
      pVirtualMachineState->PopMonitor()->Unlock( pFinalMethod->GetFullName().ToUtf8String().c_str() );
    }

  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  boost::intrusive_ptr<IJavaVariableType> pRetVal = nullptr;
  if ( parsedType.returnType.At( 0 ) != c_JavaTypeSpecifierVoid )
  {
    pRetVal = pVirtualMachineState->PopOperand();
  }

  pVirtualMachineState->PopOperand();
  if ( nullptr != pRetVal )
  {
    pVirtualMachineState->PushOperand( pRetVal );
  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  return e_IncreaseCallStackDepth::No;
}

bool BasicExecutionEngine::IsSuperClassOfCurrentClass( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, boost::intrusive_ptr<JavaString> pClassName ) const
{
  return IsSuperClassOf( pVirtualMachineState, pClassName, pVirtualMachineState->GetCurrentClass()->GetName() );
}

void BasicExecutionEngine::ExecuteOpCodeStoreLongInLocal( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, uint8_t localVariableIndex )
{
  if ( e_JavaVariableTypes::Long != pVirtualMachineState->PeekOperand()->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected long on the operand stack." );
  }

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      GetLogger()->LogDebug("Storing long %s in local variable %d", pVirtualMachineState->PeekOperand()->ToString().ToUtf8String().c_str(), localVariableIndex);
      GetLogger()->LogDebug("**Before:");
      pVirtualMachineState->LogLocalVariables();
  }
#endif

  pVirtualMachineState->SetLocalVariable( localVariableIndex, pVirtualMachineState->PopOperand() );
  //pVirtualMachineState->SetLocalVariable( localVariableIndex + 1,new ObjectReference(nullptr) );

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      GetLogger()->LogDebug("**After:");
      pVirtualMachineState->LogLocalVariables();
  }
#endif
}

void BasicExecutionEngine::ExecuteOpCodeStoreLongInLocalWithIndex( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  ExecuteOpCodeStoreLongInLocal( pVirtualMachineState, ReadByteUnsigned( pVirtualMachineState ) );
}

void BasicExecutionEngine::ExecuteOpCodePushIntImmediateByte( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  ExecuteOpCodePushInt( pVirtualMachineState, ReadByteSigned( pVirtualMachineState ) );
}

// uint8_t BasicExecutionEngine::ReadByte( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState )
// {
//   if ( !pVirtualMachineState->CanReadBytes( 1 ) )
//   {
//     throw InvalidStateException( __FUNCTION__ " - Should be able to read 1 bytes for this instruction, but the code segment is too short." );
//   }
//
//   uint8_t result = pVirtualMachineState->GetCodeSegmentStart()[ pVirtualMachineState->GetProgramCounter() ];
//   pVirtualMachineState->AdvanceProgramCounter( 1 );
//
//   return result;
// }

int8_t BasicExecutionEngine::ReadByteSigned( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  if ( !pVirtualMachineState->CanReadBytes( 1 ) )
  {
    pVirtualMachineState->LogCallStack();
    throw InvalidStateException( __FUNCTION__ " - Should be able to read 1 bytes for this instruction, but the code segment is too short." );
  }

  int8_t result = pVirtualMachineState->GetCodeSegmentStart()[ pVirtualMachineState->GetProgramCounter() ];
  pVirtualMachineState->AdvanceProgramCounter( 1 );

  return result;
}


void BasicExecutionEngine::ExecuteOpCodeNewArray( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  boost::intrusive_ptr< JavaInteger > pCount = boost::dynamic_pointer_cast<JavaInteger>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pCount )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected an operand of type int." );
  }

  if ( pCount->ToHostInt32() < 0 )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaNegativeArraySizeExceptionException );
    return;
  }

  uint8_t type = ReadByteUnsigned( pVirtualMachineState );
  ValidateArrayType( type );

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      GetLogger()->LogDebug("Creating array of type %d", static_cast<int>(type));
  }
#endif

  boost::intrusive_ptr<ObjectReference> pArray = pVirtualMachineState->CreateArray( static_cast<e_JavaArrayTypes>( type ), pCount->ToHostInt32() );

  pVirtualMachineState->PushOperand( pArray );
}

void BasicExecutionEngine::ExecuteOpCodePushFromConstantPoolWide( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  std::shared_ptr<ConstantPoolEntry> pEntry = pVirtualMachineState->GetConstantFromCurrentClass( ReadIndex( pVirtualMachineState ) );

  if ( e_ConstantPoolEntryTypeInteger == pEntry->GetType() )
  {
    pVirtualMachineState->PushOperand( pEntry->AsInteger() );
  }
  else if ( e_ConstantPoolEntryTypeFloat == pEntry->GetType() )
  {
    pVirtualMachineState->PushOperand( pEntry->AsFloat() );
  }
  else if ( e_ConstantPoolEntryTypeString == pEntry->GetType() )
  {
    pVirtualMachineState->PushOperand( pEntry->AsString() );
  }
  else if ( e_ConstantPoolEntryTypeStringReference == pEntry->GetType() )
  {
    std::shared_ptr<ConstantPoolStringReference> pStringRefRef = pEntry->AsStringReference();

    boost::intrusive_ptr<ObjectReference> pObject = pVirtualMachineState->CreateStringObject( *pVirtualMachineState->GetConstantFromCurrentClass( pStringRefRef->ToConstantPoolIndex() )->AsString() );

    pVirtualMachineState->PushOperand( pObject );
  }
  else if ( e_ConstantPoolEntryTypeClassReference == pEntry->GetType() )
  {
    boost::intrusive_ptr<ObjectReference> pClassObject = pVirtualMachineState->CreateJavaLangClassFromClassName( pEntry->AsClassReferencePointer()->GetClassName() );
    pVirtualMachineState->PushOperand( pClassObject );
  }
  else if ( e_ConstantPoolEntryTypeMethodReference == pEntry->GetType() )
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }
}

void BasicExecutionEngine::ExecuteOpCodeStoreIntoIntArray( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        pVirtualMachineState->LogOperandStack();
    }
#endif

  boost::intrusive_ptr< IJavaVariableType > pOperand = pVirtualMachineState->PopOperand();

  //  boost::dynamic_pointer_cast<JavaInteger>(pOperand);
  if ( !pOperand->IsIntegerCompatible() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected integer on operand stack." );
  }

  boost::intrusive_ptr< JavaInteger > pValue = TypeParser::UpCastToInteger( pOperand.get() );

  boost::intrusive_ptr< JavaInteger > pIndex = boost::dynamic_pointer_cast<JavaInteger>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pIndex )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected integer on operand stack." );
  }

  boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pArray )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected reference to JavaArray on operand stack." );
  }

  if ( pArray->IsNull() )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaNullPointerExceptionException );
  }

  if ( pIndex->ToHostInt32() < 0 || static_cast<size_t>( pIndex->ToHostInt32() ) > pArray->GetContainedArray()->GetNumberOfElements() )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaArrayIndexOutOfBoundsException );
  }

  pArray->GetContainedArray()->SetAt( *pIndex, *pValue );
}

void BasicExecutionEngine::ExecuteOpCodeGetField( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  std::shared_ptr<ConstantPoolFieldReference> pFieldRef = pVirtualMachineState->GetConstantFromCurrentClass( ReadIndex( pVirtualMachineState ) )->AsFieldReferencePointer();

  std::shared_ptr<FieldInfo> pFieldInfo = ResolveField( pVirtualMachineState, *pFieldRef->GetClassName(), *pFieldRef->GetName() );

  if ( nullptr == pFieldInfo )
  {
    throw InvalidStateException( __FUNCTION__ " - Could not find field." );
  }

  if ( pFieldInfo->IsStatic() )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaIncompatibleClassChangeErrorException );
    return;
  }

  if ( pVirtualMachineState->PeekOperand()->GetVariableType() == e_JavaVariableTypes::NullReference )
  {
#ifdef _DEBUG
    GetLogger()->LogDebug( "Null reference trying to get field: %s.%s", pFieldRef->GetClassName()->ToUtf8String().c_str(), pFieldRef->GetName()->ToUtf8String().c_str() );
#endif // _DEBUG

#if defined (_DEBUG)
    pVirtualMachineState->LogCallStack();
    pVirtualMachineState->LogOperandStack();
    pVirtualMachineState->LogLocalVariables();
#endif

    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaNullPointerExceptionException );
    return;
  }

  boost::intrusive_ptr<ObjectReference> pObject = boost::dynamic_pointer_cast<ObjectReference>( pVirtualMachineState->PopOperand() );

  if ( pFieldInfo->IsProtected() )
  {
    // If the field is protected (§4.6), and it is a member of a superclass of the current class, and the field is not declared in the same runtime package
    // ( §5.3 ) as the current class, then the class of objectref must be either the current class or a subclass of the current class.

    // TODO: Implement this check.
  }

  auto pFieldValue = pObject->GetContainedObject()->GetFieldByName( *pFieldInfo->GetName() );
  if ( nullptr == pFieldValue )
  {
#ifdef _DEBUG
    __asm int 3;
#endif
    throw InvalidStateException(__FUNCTION__ " - Could not find expected field on object.");
  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      GetLogger()->LogDebug("Getting Field (%s) value (%s)", pFieldInfo->GetName()->ToUtf8String().c_str(), pFieldValue->ToString().ToUtf8String().c_str());
  }
#endif // _DEBUG
  pVirtualMachineState->PushOperand( pFieldValue );
}

int16_t BasicExecutionEngine::ReadOffset( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  if ( !pVirtualMachineState->CanReadBytes( sizeof( ConstantPoolIndex ) ) )
  {
    throw InvalidStateException( __FUNCTION__ " - Not enough space in current code segment to read an index." );
  }

  uint8_t branchByte1 = pVirtualMachineState->GetCodeSegmentStart()[ pVirtualMachineState->GetProgramCounter() ];
  pVirtualMachineState->AdvanceProgramCounter( 1 );
  uint8_t branchByte2 = pVirtualMachineState->GetCodeSegmentStart()[ pVirtualMachineState->GetProgramCounter() ];
  pVirtualMachineState->AdvanceProgramCounter( 1 );

  // #ifdef _DEBUG
  //   if ( ((branchByte1 << 8) | branchByte2) == -124 )
  //   {
  //     BreakDebug(__FUNCTION__, "Test" );
  //   }
  // #endif // _DEBUG

  return static_cast<int16_t>( ( branchByte1 << 8 ) | branchByte2 );
}

void BasicExecutionEngine::ExecutedOpBranchIfEquals( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int16_t offset = ReadOffset( pVirtualMachineState );

  if ( GetIntegerFromOperandStack( pVirtualMachineState ) == 0 )
  {
    AdjustProgramCounterByOffset( pVirtualMachineState, offset );
  }
}

void BasicExecutionEngine::ExecutedOpCodeBranchIfNotEquals( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int16_t offset = ReadOffset( pVirtualMachineState );

  if ( GetIntegerFromOperandStack( pVirtualMachineState ) != 0 )
  {
    AdjustProgramCounterByOffset( pVirtualMachineState, offset );
  }
}

void BasicExecutionEngine::ExecutedOpCodeBranchIfLessThan( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int16_t offset = ReadOffset( pVirtualMachineState );

  if ( GetIntegerFromOperandStack( pVirtualMachineState ) < 0 )
  {
    AdjustProgramCounterByOffset( pVirtualMachineState, offset );
  }
}

void BasicExecutionEngine::ExecutedOpCodeBranchIfGreaterThan( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int16_t offset = ReadOffset( pVirtualMachineState );

  if ( GetIntegerFromOperandStack( pVirtualMachineState ) > 0 )
  {
    AdjustProgramCounterByOffset( pVirtualMachineState, offset );
  }
}

void BasicExecutionEngine::ExecutedOpCodeBranchIfGreaterThanOrEqual( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int16_t offset = ReadOffset( pVirtualMachineState );

  if ( GetIntegerFromOperandStack( pVirtualMachineState ) >= 0 )
  {
    AdjustProgramCounterByOffset( pVirtualMachineState, offset );
  }
}

void BasicExecutionEngine::ExecutedOpCodeBranchIfLessThanOrEqual( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int16_t offset = ReadOffset( pVirtualMachineState );

  if ( GetIntegerFromOperandStack( pVirtualMachineState ) <= 0 )
  {
    AdjustProgramCounterByOffset( pVirtualMachineState, offset );
  }
}

void BasicExecutionEngine::ExecuteOpCodeArrayLength( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  boost::intrusive_ptr<IJavaVariableType> pOperand = pVirtualMachineState->PopOperand();

  if ( pOperand->IsNull() )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaNullPointerExceptionException );
    return;
  }

  boost::intrusive_ptr<ObjectReference> pArrayRef = boost::dynamic_pointer_cast<ObjectReference>( pOperand );
  JVMX_ASSERT( e_JavaVariableTypes::Array == pArrayRef->GetVariableType() );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      GetLogger()->LogDebug("Getting length of array : %d", static_cast<int32_t>(pArrayRef->GetContainedArray()->GetNumberOfElements()));
  }
#endif // _DEBUG

  pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( static_cast<int32_t>( pArrayRef->GetContainedArray()->GetNumberOfElements() ) ) ) );
}

int BasicExecutionEngine::GetIntegerFromOperandStack( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState ) const
{
  boost::intrusive_ptr<IJavaVariableType> pOperand = pVirtualMachineState->PopOperand();

  int result = 0;
  switch ( pOperand->GetVariableType() )
  {
    case e_JavaVariableTypes::Integer:
      result = boost::dynamic_pointer_cast<JavaInteger>( pOperand )->ToHostInt32();
      break;

    case e_JavaVariableTypes::Char:
      result = boost::dynamic_pointer_cast<JavaChar>( pOperand )->ToUInt16();
      break;

    case e_JavaVariableTypes::Bool:
      result = boost::dynamic_pointer_cast<JavaBool>( pOperand )->ToUint16();
      break;

    case e_JavaVariableTypes::Byte:
      result = boost::dynamic_pointer_cast<JavaByte>( pOperand )->ToHostInt8();
      break;

    case e_JavaVariableTypes::Short:
      result = boost::dynamic_pointer_cast<JavaShort>( pOperand )->ToHostInt16();
      break;

    default:
      throw InvalidStateException( __FUNCTION__ " - Value on the operand stack was not of type integer." );
  }

  return result;
}

uint64_t BasicExecutionEngine::GetLongFromOperandStack( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  auto pOperand = pVirtualMachineState->PopOperand();

  if ( e_JavaVariableTypes::Long == pOperand->GetVariableType() )
  {
    return boost::dynamic_pointer_cast<JavaLong>( pOperand )->ToHostInt64();
  }

  if ( !pOperand->IsIntegerCompatible() )
  {
    throw InvalidStateException( __FUNCTION__ " - Value on the operand stack was not of type long or integer." );
  }

  return TypeParser::UpCastToInteger( pOperand.get() )->ToHostInt32();
}

void BasicExecutionEngine::ExecutedOpBranchIfIntegerEquals( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int16_t offset = ReadOffset( pVirtualMachineState );

  int32_t value2 = GetIntegerFromOperandStack( pVirtualMachineState );
  int32_t value1 = GetIntegerFromOperandStack( pVirtualMachineState );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      GetLogger()->LogDebug("%s - Comparing integer values %ld and %ld and jumping to offset %d if they are equal.", pVirtualMachineState->GetCurrentClassAndMethodName().ToUtf8String().c_str(), value1, value2, offset);
  }
#endif // _DEBUG

  if ( value1 == value2 )
  {
    AdjustProgramCounterByOffset( pVirtualMachineState, offset );
  }
}

void BasicExecutionEngine::ExecutedOpBranchIfIntegerNotEquals( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int16_t offset = ReadOffset( pVirtualMachineState );

  int32_t value2 = GetIntegerFromOperandStack( pVirtualMachineState );
  int32_t value1 = GetIntegerFromOperandStack( pVirtualMachineState );

  if ( value1 != value2 )
  {
    AdjustProgramCounterByOffset( pVirtualMachineState, offset );
  }
}

void BasicExecutionEngine::ExecutedOpBranchIfIntegerLessThan( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int16_t offset = ReadOffset( pVirtualMachineState );

  int32_t value2 = GetIntegerFromOperandStack( pVirtualMachineState );
  int32_t value1 = GetIntegerFromOperandStack( pVirtualMachineState );

  if ( value1 < value2 )
  {
    AdjustProgramCounterByOffset( pVirtualMachineState, offset );
  }
}

void BasicExecutionEngine::ExecutedOpBranchIfIntegerGreatherThanOrEqual( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int16_t offset = ReadOffset( pVirtualMachineState );

  int32_t value2 = GetIntegerFromOperandStack( pVirtualMachineState );
  int32_t value1 = GetIntegerFromOperandStack( pVirtualMachineState );

  if ( value1 >= value2 )
  {
    AdjustProgramCounterByOffset( pVirtualMachineState, offset );
  }
}

void BasicExecutionEngine::ExecutedOpBranchIfIntegerGreatherThan( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int16_t offset = ReadOffset( pVirtualMachineState );

  int32_t value2 = GetIntegerFromOperandStack( pVirtualMachineState );
  int32_t value1 = GetIntegerFromOperandStack( pVirtualMachineState );

  if ( value1 > value2 )
  {
    AdjustProgramCounterByOffset( pVirtualMachineState, offset );
  }
}

void BasicExecutionEngine::ExecutedOpBranchIfIntegerLessThanOrEqual( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int16_t offset = ReadOffset( pVirtualMachineState );

  int32_t value2 = GetIntegerFromOperandStack( pVirtualMachineState );
  int32_t value1 = GetIntegerFromOperandStack( pVirtualMachineState );

  if ( value1 <= value2 )
  {
    AdjustProgramCounterByOffset( pVirtualMachineState, offset );
  }
}

void BasicExecutionEngine::ExecuteOpCodeStoreReferenceInLocalWithIndex( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  ExecuteOpCodeStoreReferenceInLocal( pVirtualMachineState, ReadByteUnsigned( pVirtualMachineState ) );
}

void BasicExecutionEngine::ExecuteOpCodeStoreReferenceInLocal( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, uint8_t localVariableIndex )
{
  e_JavaVariableTypes type = pVirtualMachineState->PeekOperand()->GetVariableType();
  if ( !IsReference( type ) )
  {
#ifdef _DEBUG
    pVirtualMachineState->LogOperandStack();
    pVirtualMachineState->LogCallStack();
#endif // _DEBUG
    throw InvalidStateException( __FUNCTION__ " - Expected reference on the operand stack." );
  }

  pVirtualMachineState->SetLocalVariable( localVariableIndex, pVirtualMachineState->PopOperand() );
}

bool BasicExecutionEngine::IsReference( e_JavaVariableTypes type )
{
  return ( e_JavaVariableTypes::Object == type ) || ( e_JavaVariableTypes::Array == type ) || ( e_JavaVariableTypes::ClassReference == type ) || ( e_JavaVariableTypes::NullReference == type ) || ( e_JavaVariableTypes::ReturnAddress == type );
}

void BasicExecutionEngine::ExecuteOpCodeGoto( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  AdjustProgramCounterByOffset( pVirtualMachineState, ReadOffset( pVirtualMachineState ) );
}

void BasicExecutionEngine::AdjustProgramCounterByOffset( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, int16_t offset )
{
  pVirtualMachineState->AdvanceProgramCounter( offset - ( sizeof( int16_t ) + sizeof( int8_t ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeReturnReference( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#ifdef _DEBUG
  const size_t debugSize = pVirtualMachineState->GetOperandStackSize();
#endif // _DEBUG

  boost::intrusive_ptr<IJavaVariableType> pReference = pVirtualMachineState->PopOperand();
  if ( !IsReference( pReference->GetVariableType() ) )
  {
    throw InvalidStateException( __FUNCTION__ " - Value on the operand stack was not of type reference." );
  }

  std::shared_ptr<MethodInfo> pMethodInfo = pVirtualMachineState->GetCurrentMethodInfo();
  if ( pMethodInfo->IsSynchronised() )
  {
    pVirtualMachineState->PopMonitor()->Unlock( pMethodInfo->GetFullName().ToUtf8String().c_str() );
  }

  pVirtualMachineState->PopState();
  pVirtualMachineState->ReleaseLocalVariables();

  pVirtualMachineState->PushOperand( pReference );

#ifdef _DEBUG
  JVMX_ASSERT( pVirtualMachineState->GetOperandStackSize() == debugSize );
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeMonitorExit( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  auto pOperand = pVirtualMachineState->PopOperand();

  if ( e_JavaVariableTypes::Object == pOperand->GetVariableType() )
  {
    boost::intrusive_ptr<ObjectReference> pReference = boost::dynamic_pointer_cast<ObjectReference>( pOperand );
    if ( nullptr == pReference )
    {
      throw InvalidStateException( __FUNCTION__ " - Expected object on operand stack." );
    }

    pReference->GetContainedObject()->MonitorExit( pVirtualMachineState->GetCurrentClassAndMethodName().ToUtf8String().c_str() );
  }
  else if ( e_JavaVariableTypes::Array == pOperand->GetVariableType() )
  {
    boost::intrusive_ptr<ObjectReference> pReference = boost::dynamic_pointer_cast<ObjectReference>( pOperand );
    if ( nullptr == pReference )
    {
      throw InvalidStateException( __FUNCTION__ " - Expected array on operand stack." );
    }

    pReference->GetContainedArray()->MonitorExit( pVirtualMachineState->GetCurrentClassAndMethodName().ToUtf8String().c_str() );
  }
  else if ( e_JavaVariableTypes::ClassReference == pOperand->GetVariableType() )
  {
    boost::intrusive_ptr<JavaClassReference> pReference = boost::dynamic_pointer_cast<JavaClassReference>( pOperand );
    if ( nullptr == pReference )
    {
      throw InvalidStateException( __FUNCTION__ " - Expected array on operand stack." );
    }

    pReference->GetClassFile()->MonitorExit( pVirtualMachineState->GetCurrentClassAndMethodName().ToUtf8String().c_str() );
  }
  else
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected object or array on operand stack." );
  }
}

#ifdef _DEBUG
void BasicExecutionEngine::BreakDebug( const char *pString1, const char *pString2 )
{
  if ( strcmp( pString1, pString2 ) == 0 )
  {
    __asm int 3;
  }
}

void BasicExecutionEngine::BreakDebug( const char16_t *pString1, const char16_t *pString2 )
{
  if ( wcscmp( ( const wchar_t * )pString1, ( const wchar_t * )pString2 ) == 0 )
  {
    __asm int 3;
  }
}

void BasicExecutionEngine::BreakDebug( const uint8_t *pString1, const char *pString2 )
{
  BreakDebug( ( const char * )pString1, ( const char * )pString2 );
}

void BasicExecutionEngine::BreakDebug( const uint8_t *pString1, const uint8_t *pString2 )
{
  BreakDebug( ( const char * )pString1, ( const char * )pString2 );
}
#endif // _DEBUG

void BasicExecutionEngine::ExecuteOpCodeReturnInteger( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#ifdef _DEBUG
  const size_t debugSize = pVirtualMachineState->GetOperandStackSize();
#endif // _DEBUG

  std::shared_ptr<MethodInfo> pMethodInfo = pVirtualMachineState->GetCurrentMethodInfo();
  if ( pMethodInfo->IsSynchronised() )
  {
    pVirtualMachineState->PopMonitor()->Unlock( pMethodInfo->GetFullName().ToUtf8String().c_str() );
  }

  if ( pVirtualMachineState->HasExceptionOccurred() )
  {
    return;
  }

  boost::intrusive_ptr<IJavaVariableType> pReference = pVirtualMachineState->PopOperand();
  if ( pReference->GetVariableType() != e_JavaVariableTypes::Integer )
  {
    pReference = TypeParser::UpCastToInteger( pReference.get() );
    if ( nullptr == pReference )
    {
      throw InvalidStateException( __FUNCTION__ " - Value on the operand stack was not of type integer or convertible to integer." );
    }
  }

  pVirtualMachineState->PopState();
  pVirtualMachineState->ReleaseLocalVariables();

  pVirtualMachineState->PushOperand( pReference );

#ifdef _DEBUG
  JVMX_ASSERT( pVirtualMachineState->GetOperandStackSize() == debugSize );
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeIntegerRemainder( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int integer2 = GetIntegerFromOperandStack( pVirtualMachineState );
  int integer1 = GetIntegerFromOperandStack( pVirtualMachineState );

  pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( integer1 % integer2 ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeStoreIntegerInLocal( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, uint8_t localVariableIndex )
{
  if ( e_JavaVariableTypes::Integer != pVirtualMachineState->PeekOperand()->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected integer on the operand stack." );
  }

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      GetLogger()->LogDebug("Storing Integer %s in local variable %d", pVirtualMachineState->PeekOperand()->ToString().ToUtf8String().c_str(), localVariableIndex);
  }
#endif // _DEBUG

  pVirtualMachineState->SetLocalVariable( localVariableIndex, pVirtualMachineState->PopOperand() );
}

void BasicExecutionEngine::ExecuteOpCodeStoreIntegerInLocalWithIndex( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  ExecuteOpCodeStoreIntegerInLocal( pVirtualMachineState, ReadByteUnsigned( pVirtualMachineState ) );
}

void BasicExecutionEngine::ExecuteOpCodeLoadIntegerFromLocalWithIndex( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  ExecuteOpCodeLoadIntegerFromLocal( pVirtualMachineState, ReadByteUnsigned( pVirtualMachineState ) );
}

void BasicExecutionEngine::ExecuteOpCodeLoadIntegerFromLocal( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, uint8_t localVariableIndex )
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        GetLogger()->LogDebug("Inside iload_(%d)...", (int)localVariableIndex);
        pVirtualMachineState->LogLocalVariables();
    }
#endif

  auto pLocalValue = pVirtualMachineState->GetLocalVariable( localVariableIndex );
  if ( !pLocalValue->IsIntegerCompatible() )
  {
    pVirtualMachineState->LogLocalVariables();
    throw InvalidStateException( __FUNCTION__ " - Expected integer in local variable." );
  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      GetLogger()->LogDebug("Loading Integer %s from local variable %d", pVirtualMachineState->GetLocalVariable(localVariableIndex)->ToString().ToUtf8String().c_str(), localVariableIndex);
  }
#endif // _DEBUG

  pVirtualMachineState->PushOperand( TypeParser::UpCastToInteger( pVirtualMachineState->GetLocalVariable( localVariableIndex ).get() ) );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      pVirtualMachineState->LogOperandStack();
  }
#endif
}

void BasicExecutionEngine::ExecuteOpCodeLoadReferenceFromArray( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  if ( e_JavaVariableTypes::Integer != pVirtualMachineState->PeekOperand()->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected integer on the operand stack." );
  }

  boost::intrusive_ptr< JavaInteger > pIndex = boost::dynamic_pointer_cast<JavaInteger>( pVirtualMachineState->PopOperand() );

  if ( e_JavaVariableTypes::Array != pVirtualMachineState->PeekOperand()->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected Array on the operand stack." );
  }

  boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>( pVirtualMachineState->PopOperand() );

  if ( nullptr == pArray )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected reference to JavaArray on operand stack." );
  }

  if ( pArray->IsNull() )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaNullPointerExceptionException );
    return;
  }

  if ( pIndex->ToHostInt32() < 0 || pIndex->ToHostInt32() > static_cast<int32_t>( pArray->GetContainedArray()->GetNumberOfElements() ) )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaArrayIndexOutOfBoundsException );
    return;
  }

  IJavaVariableType *pTempReference = pArray->GetContainedArray()->At( pIndex->ToHostInt32() );
  boost::intrusive_ptr<ObjectReference> ref = new ObjectReference( *dynamic_cast<const ObjectReference *>( pTempReference ) );

  pVirtualMachineState->PushOperand( ref );
}

void BasicExecutionEngine::ExecuteOpCodeLoadCharacterFromArray( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        pVirtualMachineState->LogOperandStack();
    }
#endif

  if ( e_JavaVariableTypes::Integer != pVirtualMachineState->PeekOperand()->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected integer on the operand stack." );
  }

  boost::intrusive_ptr< JavaInteger > pIndex = boost::dynamic_pointer_cast<JavaInteger>( pVirtualMachineState->PopOperand() );

  if ( e_JavaVariableTypes::Array != pVirtualMachineState->PeekOperand()->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected Array on the operand stack." );
  }

  boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>( pVirtualMachineState->PopOperand() );

  if ( nullptr == pArray )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected reference to JavaArray on operand stack." );
  }

  if ( pArray->IsNull() )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaNullPointerExceptionException );
    return;
  }

  if ( pIndex->ToHostInt32() < 0 || pIndex->ToHostInt32() > static_cast<int32_t>( pArray->GetContainedArray()->GetNumberOfElements() ) )
  {
#ifdef _DEBUG
    pVirtualMachineState->LogCallStack();
#endif // _DEBUG

    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaArrayIndexOutOfBoundsException );
    return;
  }

  JVMX_ASSERT( pArray->GetContainedArray()->GetContainedType() == e_JavaArrayTypes::Char );

  const JavaChar *pChar = dynamic_cast<const JavaChar *>( pArray->GetContainedArray()->At( pIndex->ToHostInt32() ) );
  if ( nullptr == pChar )
  {
    throw InvalidStateException( __FUNCTION__ " - Could not convert from array contained type to char." );
  }

  pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromChar( *pChar ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeLoadFloatFromLocalWithIndex( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  ExecuteOpLoadFloatFromLocal( pVirtualMachineState, ReadByteUnsigned( pVirtualMachineState ) );
}

void BasicExecutionEngine::ExecuteOpLoadFloatFromLocal( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, uint8_t localVariableIndex )
{
  if ( e_JavaVariableTypes::Float != pVirtualMachineState->GetLocalVariable( localVariableIndex )->GetVariableType() )
  {
    pVirtualMachineState->LogLocalVariables();
    throw InvalidStateException( __FUNCTION__ " - Expected float in local variable." );
  }

  pVirtualMachineState->PushOperand( pVirtualMachineState->GetLocalVariable( localVariableIndex ) );
}

void BasicExecutionEngine::ExecuteOpCodePushFloatConstant( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, float value )
{
  pVirtualMachineState->PushOperand( new JavaFloat( JavaFloat::FromHostFloat( value ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeFloatComparisonL( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( CompareFloatValues( pVirtualMachineState, -1 ) ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeFloatComparisonG( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( CompareFloatValues( pVirtualMachineState, 1 ) ) ) );
}

int BasicExecutionEngine::CompareFloatValues( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, int nanResult )
{
  boost::intrusive_ptr<JavaFloat> pFloat2 = boost::dynamic_pointer_cast<JavaFloat>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pFloat2 )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected first operand to be float type." );
  }

  boost::intrusive_ptr<JavaFloat> pFloat1 = boost::dynamic_pointer_cast<JavaFloat>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pFloat1 )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected second operand to be float type." );
  }

  int32_t result = 0;
  if ( _isnan( pFloat1->ToHostFloat() ) || _isnan( pFloat2->ToHostFloat() ) )
  {
    result = nanResult;
  }
  else if ( *pFloat1 > *pFloat2 )
  {
    result = 1;
  }
  else if ( *pFloat1 < *pFloat2 )
  {
    result = -1;
  }
  else
  {
    result = 0;
  }

  return result;
}

int BasicExecutionEngine::CompareDoubleValues( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, int nanResult )
{
  boost::intrusive_ptr<JavaDouble> pDouble2 = boost::dynamic_pointer_cast<JavaDouble>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pDouble2 )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected first operand to be double type." );
  }

  boost::intrusive_ptr<JavaDouble> pDouble1 = boost::dynamic_pointer_cast<JavaDouble>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pDouble1 )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected second operand to be double type." );
  }

  int32_t result = 0;
  if ( _isnan( pDouble1->ToHostDouble() ) || _isnan( pDouble2->ToHostDouble() ) )
  {
    result = nanResult;
  }
  else if ( *pDouble1 > *pDouble2 )
  {
    result = 1;
  }
  else if ( *pDouble1 < *pDouble2 )
  {
    result = -1;
  }
  else
  {
    result = 0;
  }

  return result;
}

void BasicExecutionEngine::ExecuteOpCodePutField( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        GetLogger()->LogDebug(__FUNCTION__ ":");
        pVirtualMachineState->LogOperandStack();
    }
#endif // _DEBUG

  ConstantPoolIndex index = ReadIndex( pVirtualMachineState );

  std::shared_ptr<ConstantPoolFieldReference> pFieldReference = pVirtualMachineState->GetConstantFromCurrentClass( index )->AsFieldReferencePointer();
  if ( nullptr == pFieldReference )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected constant to be a field reference" );
  }

  std::shared_ptr<FieldInfo> pFieldInfo = ResolveField( pVirtualMachineState, *pFieldReference->GetClassName(), *pFieldReference->GetName() );

  boost::intrusive_ptr<IJavaVariableType> pValue = pVirtualMachineState->PopOperand();
  boost::intrusive_ptr<ObjectReference> pObject = boost::dynamic_pointer_cast<ObjectReference>( pVirtualMachineState->PopOperand() );

  if ( nullptr == pObject || pObject->IsNull() )
  {
#ifdef _DEBUG
    pVirtualMachineState->LogCallStack();
    pVirtualMachineState->LogOperandStack();
    pVirtualMachineState->LogLocalVariables();
#endif // _DEBUG

    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaNullPointerExceptionException );
    return;
  }

  if ( pFieldInfo->IsStatic() )
  {
#ifdef _DEBUG
    pVirtualMachineState->LogCallStack();
    pVirtualMachineState->LogOperandStack();
    pVirtualMachineState->LogLocalVariables();
#endif // _DEBUG

    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaIncompatibleClassChangeErrorException );
    return;
  }

  if ( pFieldInfo->IsFinal() )
  {
    // If the field is final, it must be declared in the current class, and the instruction must occur in an instance
    if ( *pFieldReference->GetClassName() != pVirtualMachineState->GetCurrentClassName() && c_InstanceInitialisationMethodName != pVirtualMachineState->GetCurrentMethodName() )
    {
#ifdef _DEBUG
      pVirtualMachineState->LogCallStack();
      pVirtualMachineState->LogOperandStack();
      pVirtualMachineState->LogLocalVariables();
#endif // _DEBUG

      HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaIllegalAccessErrorException );
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
    pObject->GetContainedObject()->SetField( *pFieldInfo->GetName(), pValue );
  }
  catch ( ... )
  {
#ifdef _DEBUG
    pVirtualMachineState->LogCallStack();
#endif // _DEBUG
    throw;
  }
}

bool BasicExecutionEngine::AreTypesCompatibile( boost::intrusive_ptr<JavaString> referenceType, boost::intrusive_ptr<JavaString> valueType )
{
  if ( referenceType == valueType )
  {
    return true;
  }

  switch ( referenceType->At( 0 ) )
  {
    case c_JavaTypeSpecifierBool:
    case c_JavaTypeSpecifierByte:
    case c_JavaTypeSpecifierChar:
    case c_JavaTypeSpecifierShort:
    case c_JavaTypeSpecifierInteger:
      return c_JavaTypeSpecifierInteger == valueType->At( 0 );
      break;

    case c_JavaTypeSpecifierLong:
      return c_JavaTypeSpecifierLong == valueType->At( 0 );
      break;

    case c_JavaTypeSpecifierFloat:
      return c_JavaTypeSpecifierFloat == valueType->At( 0 );
      break;

    case c_JavaTypeSpecifierDouble:
      return c_JavaTypeSpecifierDouble == valueType->At( 0 );
      break;

    case c_JavaTypeSpecifierArray:
      //TODO: This isn't quite right.
      return c_JavaTypeSpecifierArray == valueType->At( 0 );

    case c_JavaTypeSpecifierReference:
      //TODO: This isn't quite right.
      return c_JavaTypeSpecifierReference == valueType->At( 0 );
      break;

    default:
      throw UnsupportedTypeException( __FUNCTION__ " - Type not supported." );
      break;
  }

  return false;
}

bool BasicExecutionEngine::IsSuperClassOf( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, boost::intrusive_ptr<JavaString> pPossibleSuperClassName, boost::intrusive_ptr<JavaString> pDerivedClassName ) const
{
  return HelperClasses::IsSuperClassOf(pVirtualMachineState, pPossibleSuperClassName, pDerivedClassName);
}

void BasicExecutionEngine::ExecuteOpCodeBranchIfNotNull( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int16_t offset = ReadOffset( pVirtualMachineState );

  boost::intrusive_ptr<IJavaVariableType> pReference = pVirtualMachineState->PopOperand();
  if ( !pReference->IsNull() )
  {
    AdjustProgramCounterByOffset( pVirtualMachineState, offset );
  }
}

void BasicExecutionEngine::ExecuteOpCodeIntegerAdd( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        pVirtualMachineState->LogOperandStack();
    }
#endif // _DEBUG

  int integer2 = GetIntegerFromOperandStack( pVirtualMachineState );
  int integer1 = GetIntegerFromOperandStack( pVirtualMachineState );

  pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( integer1 + integer2 ) ) );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeIntegerMultiply( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        pVirtualMachineState->LogOperandStack();
    }
#endif // _DEBUG

  int integer2 = GetIntegerFromOperandStack( pVirtualMachineState );
  int integer1 = GetIntegerFromOperandStack( pVirtualMachineState );

  pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( integer1 * integer2 ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeIntegerSubtract( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int integer2 = GetIntegerFromOperandStack( pVirtualMachineState );
  int integer1 = GetIntegerFromOperandStack( pVirtualMachineState );

  pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( integer1 - integer2 ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeStoreIntoCharArray( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  boost::intrusive_ptr< JavaInteger > pValue = nullptr;
  boost::intrusive_ptr<IJavaVariableType> pThirdArgument = pVirtualMachineState->PopOperand();
  if ( e_JavaVariableTypes::Char == pThirdArgument->GetVariableType() )
  {
    pValue = new JavaInteger( JavaInteger::FromHostInt32( boost::dynamic_pointer_cast<JavaChar>( pThirdArgument )->ToUInt16() ) );
  }
  else
  {
    pValue = boost::dynamic_pointer_cast<JavaInteger>( pThirdArgument );
    if ( nullptr == pValue )
    {
      throw InvalidArgumentException( __FUNCTION__ " - Expected third operand to be integer type." );
    }
  }

  StoreIntoArray( pVirtualMachineState, pValue );
}

void BasicExecutionEngine::ExecuteOpCodeStoreIntoByteArray( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  boost::intrusive_ptr< JavaInteger > pValue = nullptr;
  boost::intrusive_ptr<IJavaVariableType> pThirdArgument = pVirtualMachineState->PopOperand();
  if ( e_JavaVariableTypes::Byte == pThirdArgument->GetVariableType() )
  {
    uint8_t value = boost::dynamic_pointer_cast<JavaByte>( pThirdArgument )->ToHostInt8();
    pValue = new JavaInteger( JavaInteger::FromHostInt32( value ) );
  }
  else
  {
    pValue = boost::dynamic_pointer_cast<JavaInteger>( pThirdArgument );
    if ( nullptr == pValue )
    {
      throw InvalidArgumentException( __FUNCTION__ " - Expected third operand to be integer type." );
    }
  }

  StoreIntoArray( pVirtualMachineState, pValue );
}

void BasicExecutionEngine::ExecuteOpCodeStoreIntoReferenceArray( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#if defined _DEBUG && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        pVirtualMachineState->LogCallStack();
        pVirtualMachineState->LogOperandStack();
    }
#endif // _DEBUG

  boost::intrusive_ptr<IJavaVariableType> pValue = pVirtualMachineState->PopOperand();
  if ( !pValue->IsReferenceType() )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected third operand to be reference type." );
  }

  boost::intrusive_ptr< JavaInteger > pIndex = boost::dynamic_pointer_cast<JavaInteger>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pIndex )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected second operand to be integer type." );
  }

  boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pArray )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected reference to JavaArray on operand stack." );
  }

  if ( pArray->IsNull() )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaNullPointerExceptionException );
    return;
  }

  if ( pIndex->ToHostInt32() < 0 || static_cast<size_t>( pIndex->ToHostInt32() ) > pArray->GetContainedArray()->GetNumberOfElements() )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaArrayIndexOutOfBoundsException );
  }

  pArray->GetContainedArray()->SetAt( *pIndex, pValue.get() );
}

void BasicExecutionEngine::ExecuteOpCodeTableSwitch( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  intptr_t startingAddress = pVirtualMachineState->GetProgramCounter() - 1; // -1 for the size of the switch instruction.

  intptr_t paddingLength = 4 - ( pVirtualMachineState->GetProgramCounter() % 4 );

  JVMX_ASSERT( paddingLength < 4 && paddingLength >= 0 );

  for ( int i = 0; i < paddingLength; ++ i )
  {
    uint8_t readByte = ReadByteUnsigned( pVirtualMachineState );
    readByte += 0;
  }

  JVMX_ASSERT( ( pVirtualMachineState->GetProgramCounter() % 4 ) == 0 );

  int32_t deflt = Read32BitOffset( pVirtualMachineState );
  int32_t low = Read32BitOffset( pVirtualMachineState );
  int32_t high = Read32BitOffset( pVirtualMachineState );

  int32_t offsetCount = high - low + 1;

  std::vector<int32_t> jumpTable( offsetCount );
  for ( int i = 0; i < offsetCount; ++ i )
  {
    jumpTable[ i ] = Read32BitOffset( pVirtualMachineState );
  }

  boost::intrusive_ptr< JavaInteger > pIndex = boost::dynamic_pointer_cast<JavaInteger>( pVirtualMachineState->PopOperand() );

  intptr_t targetAddress = 0;
  if ( pIndex->ToHostInt32() < low || pIndex->ToHostInt32() > high )
  {
    targetAddress = startingAddress + deflt;
  }
  else
  {
    int32_t jumpTableIndex = pIndex->ToHostInt32() - low;
    targetAddress = startingAddress + jumpTable[ jumpTableIndex ];
  }

  pVirtualMachineState->AdvanceProgramCounter( targetAddress - pVirtualMachineState->GetProgramCounter() );
}

uint8_t BasicExecutionEngine::ReadByteUnsigned( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  return static_cast<uint8_t>( ReadByteSigned( pVirtualMachineState ) );
}

int32_t BasicExecutionEngine::Read32BitOffset( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  uint8_t byte1 = ReadByteUnsigned( pVirtualMachineState );
  uint8_t byte2 = ReadByteUnsigned( pVirtualMachineState );
  uint8_t byte3 = ReadByteUnsigned( pVirtualMachineState );
  uint8_t byte4 = ReadByteUnsigned( pVirtualMachineState );

  return ( byte1 << 24 ) | ( byte2 << 16 ) | ( byte3 << 8 ) | byte4;
}

void BasicExecutionEngine::ExecuteOpCodeConvertIntegerToFloat( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  boost::intrusive_ptr< JavaInteger > pInteger1 = boost::dynamic_pointer_cast<JavaInteger>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pInteger1 )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected first operand to be integer type." );
  }

  pVirtualMachineState->PushOperand( new JavaFloat( JavaFloat::FromHostFloat( static_cast<float>( pInteger1->ToHostInt32() ) ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeConvertIntegerToDouble( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  boost::intrusive_ptr< JavaInteger > pInteger1 = boost::dynamic_pointer_cast<JavaInteger>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pInteger1 )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected first operand to be integer type." );
  }

  pVirtualMachineState->PushOperand( new JavaDouble( JavaDouble::FromHostDouble( static_cast<double>( pInteger1->ToHostInt32() ) ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeFloatMultiply( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  boost::intrusive_ptr<JavaFloat> pFloat2 = boost::dynamic_pointer_cast<JavaFloat>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pFloat2 )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected first operand to be float type." );
  }

  boost::intrusive_ptr<JavaFloat> pFloat1 = boost::dynamic_pointer_cast<JavaFloat>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pFloat1 )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected second operand to be float type." );
  }

  pVirtualMachineState->PushOperand( new JavaFloat( JavaFloat::FromHostFloat( pFloat1->ToHostFloat() * pFloat2->ToHostFloat() ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeConvertFloatToInteger( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  boost::intrusive_ptr<JavaFloat> pFloat = boost::dynamic_pointer_cast<JavaFloat>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pFloat )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected first operand to be float type." );
  }

  boost::intrusive_ptr< JavaInteger > result = new JavaInteger( JavaInteger::FromDefault() );
  if ( pFloat->IsNan() )
  {
    result = new JavaInteger( JavaInteger::FromHostInt32( 0 ) );
  }
  else
  {
    result = new JavaInteger( JavaInteger::FromHostInt32( static_cast<int32_t>( pFloat->ToHostFloat() ) ) );
  }

  pVirtualMachineState->PushOperand( result );
}

std::shared_ptr<MethodInfo> BasicExecutionEngine::IdentifyVirtualMethodToCall( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, std::shared_ptr<MethodInfo> pMethodInfo, boost::intrusive_ptr<ObjectReference> pObject )
{
#ifdef _DEBUG
  std::shared_ptr<MethodInfo> pOldMethod = pMethodInfo;
#endif // _DEBUG

  JavaString methodName = *( pMethodInfo->GetName() );
  JavaString methodType = *( pMethodInfo->GetType() );
  std::shared_ptr<MethodInfo> pMethodToExecute = pObject->GetContainedObject()->GetClass()->GetMethodByNameAndType( methodName, methodType );
  std::shared_ptr<JavaClass> pClass = pObject->GetContainedObject()->GetClass();

  if ( nullptr == pMethodToExecute )
  {
    std::shared_ptr<JavaClass> pSuperClass = pClass->GetSuperClass();
    while ( nullptr != pSuperClass )
    {
      pMethodToExecute = pSuperClass->GetMethodByNameAndType( methodName, methodType );
      if ( nullptr != pMethodToExecute )
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
  if ( nullptr == pMethodToExecute )
  {
    BreakDebug( "a", "a" );
  }
#endif // _DEBUG

  return pMethodToExecute;
}

void BasicExecutionEngine::ExecuteOpCodeBranchIfNull( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int16_t offset = ReadOffset( pVirtualMachineState );

  boost::intrusive_ptr<IJavaVariableType> pReference = pVirtualMachineState->PopOperand();
  if ( pReference->IsNull() )
  {
    AdjustProgramCounterByOffset( pVirtualMachineState, offset );
  }
}

void BasicExecutionEngine::ExecuteOpCodeIsInstanceOf( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  boost::intrusive_ptr<IJavaVariableType> pOperand = pVirtualMachineState->PopOperand();
  ConstantPoolIndex index = ReadIndex( pVirtualMachineState );

  JVMX_ASSERT( pOperand->IsReferenceType() );

  if ( e_JavaVariableTypes::Object != pOperand->GetVariableType() )
  {
    if ( e_JavaVariableTypes::NullReference == pOperand->GetVariableType() )
    {
      pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( 0 ) ) );
      return;
    }

    if ( e_JavaVariableTypes::ClassReference != pOperand->GetVariableType() )
    {
      if ( e_JavaVariableTypes::Array != pOperand->GetVariableType() )
      {
        throw InvalidArgumentException( __FUNCTION__ " - Expected first operand to be an object or a null reference type." );
      }
    }
  }

  std::shared_ptr<JavaClass> pResolvedClass = ResolveClassFromIndex( pVirtualMachineState, index );
  if ( nullptr == pResolvedClass )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Could not resolve class reference." );
  }

  if ( e_JavaVariableTypes::Object == pOperand->GetVariableType() )
  {
    boost::intrusive_ptr<ObjectReference> pObject = boost::dynamic_pointer_cast<ObjectReference>( pOperand );
    if ( !pResolvedClass->IsInterface() )
    {
      pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( IsInstanceOf( pVirtualMachineState, pResolvedClass->GetName(), pObject->GetContainedObject()->GetClass()->GetName() ) ? 1 : 0 ) ) );
      return;
    }
    else
    {
      bool bFound = DoesClassImplementInterface( pVirtualMachineState, pObject->GetContainedObject()->GetClass(), pResolvedClass->GetName() );
      pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( bFound ? 1 : 0 ) ) );
      return;
    }
  }
  else if ( e_JavaVariableTypes::ClassReference == pOperand->GetVariableType() )
  {
    // This should not happen.
    JVMX_ASSERT( false );
  }
  else if ( e_JavaVariableTypes::Array == pOperand->GetVariableType() )
  {
    boost::intrusive_ptr<ObjectReference> pOperandAsArray = boost::dynamic_pointer_cast<ObjectReference>( pOperand );
    if ( nullptr == pOperandAsArray )
    {
      throw InvalidStateException( __FUNCTION__ " - Expected an array." );
    }

    if ( !pResolvedClass->IsInterface() )
    {
      if ( *pResolvedClass->GetName() != JavaString::FromCString( "java/lang/Object" ) )
      {
        throw InvalidStateException( __FUNCTION__ " - Expected argument of to be of type Object." );
      }

      pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( 1 ) ) );
      return;
    }
    else
    {
      if ( *pResolvedClass->GetName() == JavaString::FromCString( "java/lang/Object" ) ||
           *pResolvedClass->GetName() == JavaString::FromCString( "java/lang/Cloneable" ) ||
           *pResolvedClass->GetName() == JavaString::FromCString( "java/io/Serializable" )
         )
      {
        pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( 1 ) ) );
        return;
      }
    }

    // TODO: If T is an array type TC[], that is, an array of components of type TC, then one of the following must be true:
    // - TC and SC are the same primitive type.
    // - TC and SC are reference types, and type SC can be cast to TC by these runtime rules.
    pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( 0 ) ) );
    return;
  }

  JVMX_ASSERT( false );
  pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( 0 ) ) );
}

bool BasicExecutionEngine::IsInstanceOf( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, boost::intrusive_ptr<JavaString> pPossibleSuperClassName, boost::intrusive_ptr<JavaString> pDerivedClassName ) const
{
  if ( *pPossibleSuperClassName == *pDerivedClassName )
  {
    return true;
  }

  return IsSuperClassOf( pVirtualMachineState, pPossibleSuperClassName, pDerivedClassName );
}

void BasicExecutionEngine::ExecuteOpCodeCheckCast( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  boost::intrusive_ptr<IJavaVariableType> pOperand = pVirtualMachineState->PeekOperand();
  JVMX_ASSERT( pOperand->IsReferenceType() );

  // NB: Need to do this in all cases, so we we can't skip if we find a null reference.
  ConstantPoolIndex classIndex = ReadIndex( pVirtualMachineState );

  if ( e_JavaVariableTypes::NullReference == pOperand->GetVariableType() )
  {
    return;
  }

  if ( !pOperand->IsReferenceType() )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected first operand to be an a reference type." );
  }

  std::shared_ptr<ConstantPoolClassReference> pClassRef = pVirtualMachineState->GetConstantFromCurrentClass( classIndex )->AsClassReferencePointer();
  if ( nullptr == pClassRef )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected index to be a reference to a class/interface/array." );
  }

  std::shared_ptr<JavaClass> pResolvedClass = nullptr;
  bool isClassRefArray = false;

  JavaString className = *pClassRef->GetClassName();
  if ( c_JavaTypeSpecifierArray == className.At( 0 ) )
  {
    while ( c_JavaTypeSpecifierArray == className.At( 0 ) )
    {
      // Remove the ['s at the beginning
      className = className.SubString( 1, className.GetLengthInCodePoints() - 1 );
    }

    if ( c_JavaTypeSpecifierReference == className.At( 0 ) )
    {
      // Remove the L and ; around the class Name
      className = className.SubString( 1, className.GetLengthInCodePoints() - 2 );
    }

    isClassRefArray = true;
  }
  else
  {
    pResolvedClass = ResolveClass( pVirtualMachineState, className );
    if ( nullptr == pResolvedClass )
    {
      throw InvalidArgumentException( __FUNCTION__ " - Could not resolve class reference." );
    }
  }

  if ( e_JavaVariableTypes::Object == pOperand->GetVariableType() )
  {
    CheckCastForObjects( pVirtualMachineState, pOperand, pResolvedClass );
    return;
  }
  else if ( e_JavaVariableTypes::ClassReference == pOperand->GetVariableType() )
  {
    // This should not happen
    JVMX_ASSERT( false );
    //CheckCastForReferences( pOperand, pResolvedClass );
    return;
  }
  else if ( e_JavaVariableTypes::Array == pOperand->GetVariableType() )
  {
    CheckCastForArrays( pVirtualMachineState, pOperand, pResolvedClass, isClassRefArray, new JavaString( className ) );
    return;
  }

  return;
}

void BasicExecutionEngine::ExecuteOpCodePopOperandStack( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  pVirtualMachineState->PopOperand();
}

// TODO: Debug here
void BasicExecutionEngine::ExecuteOpCodeBranchIfReferencesAreEqual( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int16_t offset = ReadOffset( pVirtualMachineState );

  boost::intrusive_ptr<IJavaVariableType> pValue1 = pVirtualMachineState->PopOperand();
  if ( !pValue1->IsReferenceType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected first operand to be an a reference type." );
  }

  boost::intrusive_ptr<IJavaVariableType> pValue2 = pVirtualMachineState->PopOperand();
  if ( !pValue2->IsReferenceType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected second operand to be an a reference type." );
  }

  bool areValuesEqual = false;
  if ( pValue2->GetVariableType() == pValue1->GetVariableType() )
  {
    areValuesEqual = boost::dynamic_pointer_cast<ObjectReference>( pValue1 )->GetIndex() == boost::dynamic_pointer_cast<ObjectReference>( pValue2 )->GetIndex();
  }

  if ( areValuesEqual )
  {
    AdjustProgramCounterByOffset( pVirtualMachineState, offset );
  }
}

void BasicExecutionEngine::ExecuteOpCodeBranchIfReferencesAreNotEqual( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int16_t offset = ReadOffset( pVirtualMachineState );

  boost::intrusive_ptr<IJavaVariableType> pValue1 = pVirtualMachineState->PopOperand();
  if ( !pValue1->IsReferenceType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected first operand to be an a reference type." );
  }

  boost::intrusive_ptr<IJavaVariableType> pValue2 = pVirtualMachineState->PopOperand();
  if ( !pValue2->IsReferenceType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected second operand to be an a reference type." );
  }

  bool areValuesEqual = false;
  if ( pValue2->GetVariableType() == pValue1->GetVariableType() )
  {
    areValuesEqual = boost::dynamic_pointer_cast<ObjectReference>( pValue1 )->GetIndex() == boost::dynamic_pointer_cast<ObjectReference>( pValue2 )->GetIndex();
    //areValuesEqual = (pValue2-> == *pValue1);
  }

  if ( !areValuesEqual )
  {
    AdjustProgramCounterByOffset( pVirtualMachineState, offset );
  }
}

void BasicExecutionEngine::ExecuteOpCodeIncrementLocalVariable( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  uint8_t index = ReadByteUnsigned( pVirtualMachineState );
  int8_t constantValue = ReadByteSigned( pVirtualMachineState );

  boost::intrusive_ptr< JavaInteger > pIntValue = boost::dynamic_pointer_cast<JavaInteger>( pVirtualMachineState->GetLocalVariable( index ) );
  if ( nullptr == pIntValue )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected integer in local variable" );
  }

  int32_t finalValue = pIntValue->ToHostInt32() + constantValue;

#ifdef _DEBUG
  if ( finalValue < pIntValue->ToHostInt32() && constantValue > 0 )
  {
    JVMX_ASSERT( false );
  }
#endif // _DEBUG

  pVirtualMachineState->SetLocalVariable( index, new JavaInteger( JavaInteger::FromHostInt32( finalValue ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeNegateInteger( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int32_t value = GetIntegerFromOperandStack( pVirtualMachineState );
  pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( value * -1 ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeConvertIntegerToChar( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  boost::intrusive_ptr< JavaInteger > pInteger1 = boost::dynamic_pointer_cast<JavaInteger>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pInteger1 )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected first operand to be integer type." );
  }

  uint16_t characterValue = static_cast<uint16_t>( pInteger1->ToHostInt32() );

  //boost::intrusive_ptr<JavaChar> pFinalChar = new JavaChar( JavaChar::FromUInt16( characterValue & 0x00FF ) );
  boost::intrusive_ptr<JavaChar> pFinalChar = new JavaChar( JavaChar::FromUInt16( characterValue & 0xFFFF ) );
  /*
  #ifdef _DEBUG
    if ( pFinalChar->ToUInt16() > UCHAR_MAX )
    {
      _asm int 3;
    }
  #endif // _DEBUG
  */
  pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromChar( *pFinalChar ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeShiftIntegerLeft( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int32_t value2 = GetIntegerFromOperandStack( pVirtualMachineState );
  int32_t value1 = GetIntegerFromOperandStack( pVirtualMachineState );

  pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( value1 << ( value2 & 0x1F ) ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeShiftIntegerRightArithmetic( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int32_t value2 = GetIntegerFromOperandStack( pVirtualMachineState );
  int32_t value1 = GetIntegerFromOperandStack( pVirtualMachineState );

  pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( value1 >> ( value2 & 0x1F ) ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeShiftIntegerRightLogical( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        pVirtualMachineState->LogOperandStack();
    }
#endif // _DEBUG

  int32_t value2 = GetIntegerFromOperandStack( pVirtualMachineState );
  int32_t value1 = GetIntegerFromOperandStack( pVirtualMachineState );

  value2 = ( value2 & 0x1F ); // Get bottom 5 bits.

  // see this: http://stackoverflow.com/questions/5253194/implementing-logical-right-shift-in-c

  int size = sizeof( int32_t ) * 8;
  int32_t result = ( value1 >> value2 ) & ~( ( ( 0x1 << ( size - 1 ) ) >> value2 ) << 1 );

  JVMX_ASSERT( result == ( ( uint32_t )value1 ) >> value2 );

  pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( result ) ) );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

bool BasicExecutionEngine::DoesClassImplementInterface( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, 
                                                        std::shared_ptr<JavaClass> pClass, 
                                                        boost::intrusive_ptr<JavaString> nameOfInterface )
{
  return HelperClasses::DoesClassImplementInterface(pVirtualMachineState, pClass, nameOfInterface);
}

e_IncreaseCallStackDepth BasicExecutionEngine::ExecuteOpCodeInvokeInterfaceMethod( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  ConstantPoolIndex methodIndex = ReadIndex( pVirtualMachineState );
  uint8_t count = ReadByteUnsigned( pVirtualMachineState );
  ReadByteUnsigned( pVirtualMachineState );

  std::shared_ptr<ConstantPoolEntry> pConstant = pVirtualMachineState->GetConstantFromCurrentClass( methodIndex );
  if ( pConstant->GetType() != e_ConstantPoolEntryTypeInterfaceMethodReference )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected Method reference." );
  }

  std::shared_ptr<ConstantPoolMethodReference> pMethodRef = pConstant->AsMethodReference();

  auto params = TypeParser::ParseMethodType( *pMethodRef->GetType() );

  boost::intrusive_ptr<ObjectReference> pObject = boost::dynamic_pointer_cast<ObjectReference>( pVirtualMachineState->PeekOperandFromBack( static_cast< uint8_t >( params.parameters.size() ) + 1 ) );
  if ( nullptr == pObject )
  {
#if defined (_DEBUG)
    pVirtualMachineState->LogCallStack();
    pVirtualMachineState->LogOperandStack();
    pVirtualMachineState->LogLocalVariables();
#endif

    throw InvalidStateException( __FUNCTION__ " - Expected object pointer to be valid." );
  }

  if ( pObject->IsNull() )
  {
#if defined (_DEBUG)
    pVirtualMachineState->LogCallStack();
    pVirtualMachineState->LogOperandStack();
    pVirtualMachineState->LogLocalVariables();
#endif

    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaNullPointerExceptionException );
    return e_IncreaseCallStackDepth::No;
  }

  std::shared_ptr<MethodInfo> pMethodInfo = pVirtualMachineState->ResolveMethodOnClass( pObject->GetContainedObject()->GetClass()->GetName(), pMethodRef.get() );
  if ( nullptr == pMethodInfo )
  {
#if defined (_DEBUG)
    pVirtualMachineState->LogCallStack();
    pVirtualMachineState->LogOperandStack();
    pVirtualMachineState->LogLocalVariables();
#endif

    GetLogger()->LogError( "Could not resolve method (%s) on class: (%s).", pMethodInfo->GetName()->ToUtf8String().c_str(), pMethodInfo->GetClass()->GetName()->ToUtf8String().c_str() );

    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaIncompatibleClassChangeErrorException );
    return e_IncreaseCallStackDepth::No;
  }

  std::vector<boost::intrusive_ptr<IJavaVariableType> > paramArray = pVirtualMachineState->PopulateParameterArrayFromOperandStack( pMethodInfo );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      GetLogger()->LogDebug("Invoking Interface Method: %s.", pMethodInfo->GetName()->ToUtf8String().c_str());
  }
#endif // _DEBUG

  return ExecuteVirtualMethodInternal( pVirtualMachineState, pMethodInfo, pObject, paramArray, e_MethodAlreadyIdentified::Yes );
}

e_IncreaseCallStackDepth BasicExecutionEngine::ExecuteVirtualMethod( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, std::shared_ptr<ConstantPoolEntry> pConstant )
{
  std::shared_ptr<ConstantPoolMethodReference> pMethodRef = pConstant->AsMethodReference();
  std::shared_ptr<MethodInfo> pMethodInfo = pVirtualMachineState->ResolveMethodOnClass( pMethodRef->GetClassName(), pMethodRef.get() );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      pVirtualMachineState->LogOperandStack();
  }
#endif

  if ( nullptr == pMethodInfo )
  {
#if defined (_DEBUG)
    pVirtualMachineState->LogCallStack();
    pVirtualMachineState->LogOperandStack();
    pVirtualMachineState->LogLocalVariables();
#endif

    GetLogger()->LogError( "Could not resolve method (%s) on class: (%s).", pMethodInfo->GetName()->ToUtf8String().c_str(), pMethodInfo->GetClass()->GetName()->ToUtf8String().c_str() );
    throw InvalidStateException( __FUNCTION__ " - Could not resolve method on class." );
  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      GetLogger()->LogDebug("Invoking Virtual Method: %s::%s.", pMethodInfo->GetClass()->GetName()->ToUtf8String().c_str(), pMethodInfo->GetName()->ToUtf8String().c_str());
      pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  std::vector<boost::intrusive_ptr<IJavaVariableType> > paramArray = pVirtualMachineState->PopulateParameterArrayFromOperandStack( pMethodInfo );

  if ( paramArray[ 0 ]->GetVariableType() == e_JavaVariableTypes::Object )
  {
    return ExecuteVirtualMethodForObject( pVirtualMachineState, paramArray, pMethodInfo );
  }
  else if ( paramArray[ 0 ]->GetVariableType() == e_JavaVariableTypes::Array )
  {
    ExecuteVirtualMethodForArray( pVirtualMachineState, paramArray, pMethodInfo );

    return e_IncreaseCallStackDepth::No;
  }
  else if ( paramArray[ 0 ]->IsNull() )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaNullPointerExceptionException );
    return e_IncreaseCallStackDepth::No;
  }

  throw InvalidStateException( __FUNCTION__ " - Object or Array Expected." );
}

void BasicExecutionEngine::ExecuteVirtualMethodForArray( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, std::vector<boost::intrusive_ptr<IJavaVariableType> > paramArray, std::shared_ptr<MethodInfo> pMethodInfo )
{
  boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>( paramArray[ 0 ] );

  if ( *pMethodInfo->GetClass()->GetName() != JavaString::FromCString( "java/lang/Object" ) )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected class name to be Object." );
  }

  if ( *pMethodInfo->GetName() == JavaString::FromCString( "clone" ) )
  {
    std::shared_ptr<IThreadManager> pThreadManager = GlobalCatalog::GetInstance().Get( "ThreadManager" );
    boost::intrusive_ptr<ObjectReference> pNewArray = pThreadManager->GetCurrentThreadState()->CreateArray( pArray->GetContainedArray()->GetContainedType(), pArray->GetContainedArray()->GetNumberOfElements() );

    pNewArray->CloneOther( *pArray );

    pVirtualMachineState->PushOperand( pNewArray );
  }
#ifdef _DEBUG
  else
  {
    //BreakDebug( __FUNCTION__, "Need to implement this" );
    BreakDebug( "a", "a" );
  }
#endif // _DEBUG
}

e_IncreaseCallStackDepth BasicExecutionEngine::ExecuteVirtualMethodForObject( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, std::vector<boost::intrusive_ptr<IJavaVariableType> > paramArray, std::shared_ptr<MethodInfo> pMethodInfo )
{
  boost::intrusive_ptr<ObjectReference> pObject = boost::dynamic_pointer_cast<ObjectReference>( paramArray[ 0 ] );

  if ( nullptr == pObject )
  {
#if defined (_DEBUG)
    pVirtualMachineState->LogCallStack();
    pVirtualMachineState->LogOperandStack();
    pVirtualMachineState->LogLocalVariables();
#endif

    throw InvalidStateException( __FUNCTION__ " - Expected object pointer to be valid." );
  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  return ExecuteVirtualMethodInternal( pVirtualMachineState, pMethodInfo, pObject, paramArray, e_MethodAlreadyIdentified::No );
}

void BasicExecutionEngine::ExecuteOpCodePushShortConstant( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int16_t value = ReadShort( pVirtualMachineState );
  pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromShort( JavaShort::FromHostInt16( value ) ) ) );
}

e_IncreaseCallStackDepth BasicExecutionEngine::ExecuteVirtualMethodInternal( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, std::shared_ptr<MethodInfo> &pMethodInfo, boost::intrusive_ptr<ObjectReference> pObject, std::vector<boost::intrusive_ptr<IJavaVariableType> > paramArray, e_MethodAlreadyIdentified methodAlreadyIdentified )
{
  if ( pMethodInfo->IsProtected() )
  {
    // Finally, if the resolved method is protected, and it is a member of a superclass of the current class, and the method is not
    // declared in the same runtime package as the current class
    std::shared_ptr<JavaClass> pSuperClass = pVirtualMachineState->GetCurrentClass()->GetSuperClass();
    if ( pSuperClass->GetMethodByName( *( pMethodInfo->GetName() ) ) )
    {
      if ( pVirtualMachineState->GetCurrentClass()->GetPackageName() != pMethodInfo->GetPackageName() )
      {
        // then the class of *objectref* must be either the current class or a subclass of the current class.
        if ( !IsInstanceOf( pVirtualMachineState, pVirtualMachineState->GetCurrentClass()->GetName(), pObject->GetContainedObject()->GetClass()->GetName() ) )
        {
          throw InvalidStateException( __FUNCTION__ " - Expected object reference to be of the current class or a subclass." );
        }
      }
    }
  }

  if ( pMethodInfo->IsStatic() )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Method to invoke was meant to be non-static." );
  }

  if ( !pMethodInfo->IsSignaturePolymorphic() )
  {
    if ( e_MethodAlreadyIdentified::No == methodAlreadyIdentified )
    {
      pMethodInfo = IdentifyVirtualMethodToCall( pVirtualMachineState, pMethodInfo, pObject );
      if ( nullptr == pMethodInfo )
      {
        HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaAbstractMethodErrorException );
        return e_IncreaseCallStackDepth::No;
      }
    }

    try
    {
      //std::shared_ptr<JavaClassFile> pFinalClass = pObject->GetClass();

      // The objectref must be followed on the operand stack by nargs argument values, where the number, type, and order of the values
      // must be consistent with the descriptor of the selected instance method.

      if ( pMethodInfo->IsSynchronised() )
      {
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
          if (pVirtualMachineState->HasUserCodeStarted())
          {
              GetLogger()->LogDebug("Virtual Method is synchronized: %s.", pMethodInfo->GetName()->ToUtf8String().c_str());
          }
#endif // _DEBUG

        pVirtualMachineState->PushMonitor( pObject->GetContainedObject()->MonitorEnter( pMethodInfo->GetFullName().ToUtf8String().c_str() ) );
      }

      if ( !pMethodInfo->IsNative() )
      {
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
          if (pVirtualMachineState->HasUserCodeStarted())
          {
              GetLogger()->LogDebug("Setting up %u local variables", pMethodInfo->GetCodeInfo()->GetLocalVariableArraySizeIncludingPassedParameters());
              pVirtualMachineState->LogOperandStack();
          }
#endif

        //If the method is not native, the nargs argument values and objectref are popped from the operand stack.
        pVirtualMachineState->PushState( *( pMethodInfo->GetClass()->GetName() ), *( pMethodInfo->GetName() ), *( pMethodInfo->GetType() ), pMethodInfo );
        pVirtualMachineState->SetupLocalVariables( pMethodInfo, pObject, paramArray );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
        if (pVirtualMachineState->HasUserCodeStarted())
        {
            pVirtualMachineState->LogLocalVariables();
        }
#endif // _DEBUG

        JVMX_ASSERT( *pMethodInfo->GetClass()->GetName() == *pObject->GetContainedObject()->GetClass()->GetName() || IsSuperClassOf( pVirtualMachineState, pMethodInfo->GetClass()->GetName(), pObject->GetContainedObject()->GetClass()->GetName() ) );

        pVirtualMachineState->SetCodeSegment( pMethodInfo->GetCodeInfo() );

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

        TypeParser::ParsedMethodType type = TypeParser::ParseMethodType( *( pMethodInfo->GetType() ) );

        // We have to push the parameters back into the stack, because we will read them off again later as integers in the lead up to the native method.
        if ( paramArray.size() > 1 )
        {
          for ( size_t ip = 1; ip < paramArray.size(); ++ ip )
          {
            pVirtualMachineState->PushOperand( paramArray[ip] );
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
        pJNI->ExecuteFunction( JavaNativeInterface::MakeJNINameWithoutArgumentDescriptor( pMethodInfo->GetClass()->GetName(), pMethodInfo->GetName() ), pMethodInfo->GetType(), pObject );
        //pVirtualMachineState->PopState();

        if ( pMethodInfo->IsSynchronised() )
        {
          pVirtualMachineState->PopMonitor()->Unlock( pMethodInfo->GetFullName().ToUtf8String().c_str() );
        }
        return e_IncreaseCallStackDepth::No;
      }
    }
    catch ( std::exception &ex )
    {
      GetLogger()->LogDebug( __FUNCTION__ " - Exception Caught:\n\t%s", ex.what() );

      pVirtualMachineState->PopState();
      pVirtualMachineState->ReleaseLocalVariables();

      throw;
    }
    catch ( ... )
    {
#ifdef _DEBUG
      GetLogger()->LogDebug( __FUNCTION__ " - Exception Caught." );
#endif // _DEBUG
      pVirtualMachineState->PopState();
      pVirtualMachineState->ReleaseLocalVariables();

      throw;
    }
  }
  else
  {
    throw NotImplementedException( __FUNCTION__ " - Not implemented yet." );
  }

  return e_IncreaseCallStackDepth::No;
}

void BasicExecutionEngine::CheckCastForObjects( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, boost::intrusive_ptr<IJavaVariableType> pOperand, std::shared_ptr<JavaClass> pResolvedClass )
{
  boost::intrusive_ptr<ObjectReference> pOperandAsObject = boost::dynamic_pointer_cast<ObjectReference>( pOperand );
  if ( !pResolvedClass->IsInterface() )
  {
    if ( !IsInstanceOf( pVirtualMachineState, pResolvedClass->GetName(), pOperandAsObject->GetContainedObject()->GetClass()->GetName() ) )
    {
      HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaClassCastException );
      return;
    }
  }
  else
  {
    if ( DoesClassImplementInterface( pVirtualMachineState, pOperandAsObject->GetContainedObject()->GetClass(), pResolvedClass->GetName() ) )
    {
      return;
    }

    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaClassCastException );
    return;
  }
}

void BasicExecutionEngine::Halt()
{
  m_Halted = true;
}

std::shared_ptr<IClassLibrary> BasicExecutionEngine::GetClassLibrary() const
{
  return GlobalCatalog::GetInstance().Get( "ClassLibrary" );
}

std::shared_ptr<ILogger> BasicExecutionEngine::GetLogger() const
{
  return GlobalCatalog::GetInstance().Get( "Logger" );
}

void BasicExecutionEngine::CheckCastForArrays( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, boost::intrusive_ptr<IJavaVariableType> pOperand, std::shared_ptr<JavaClass> pResolvedClass, bool isClassRefArray, boost::intrusive_ptr<JavaString> pClassName )
{
  boost::intrusive_ptr<ObjectReference> pOperandAsArray = boost::dynamic_pointer_cast<ObjectReference>( pOperand );
  if ( nullptr != pResolvedClass )
  {
    if ( !pResolvedClass->IsInterface() )
    {
      if ( *pResolvedClass->GetName() != JavaString::FromCString( "java/lang/Object" ) )
      {
        HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaClassCastException );
        return;
      }
    }

    if ( *pResolvedClass->GetName() != JavaString::FromCString( "java/lang/Object" ) &&
         *pResolvedClass->GetName() != JavaString::FromCString( "java/lang/Cloneable" ) &&
         *pResolvedClass->GetName() != JavaString::FromCString( "java/io/Serializable" ) )
    {
      HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaClassCastException );
      return;
    }
  }
  else
  {
    // If T is an array type TC[], that is, an array of components of type TC, then one of the following must be true:
    // - TC and SC are the same primitive type.
    // - TC and SC are reference types, and type SC can be cast to TC by recursive application of these rules.

    JVMX_ASSERT( isClassRefArray );
    if ( e_JavaArrayTypes::Reference != pOperandAsArray->GetContainedArray()->GetContainedType() )
    {
      if ( JavaArray::ConvertTypeFromChar( pClassName->At( 0 ) ) == pOperandAsArray->GetContainedArray()->GetContainedType() )
      {
        return;
      }
    }
    else
    {
      if ( 0 == pOperandAsArray->GetContainedArray()->GetNumberOfElements() || e_JavaVariableTypes::NullReference == pOperandAsArray->GetContainedArray()->At( 0 )->GetVariableType() )
      {
        // We don't know what's inside this array, but since it's empty it can still hold any object type.
        return;
      }

      const ObjectReference *pArrayElement = dynamic_cast<const ObjectReference *>( pOperandAsArray->GetContainedArray()->At( 0 ) );
      JVMX_ASSERT( nullptr != pArrayElement );
      if ( IsInstanceOf( pVirtualMachineState, pClassName, pArrayElement->GetContainedObject()->GetClass()->GetName() ) )
      {
        return;
      }

      HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaClassCastException );
    }
  }
}

void BasicExecutionEngine::ExecuteOpCodeIntegerDivide( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int integer2 = GetIntegerFromOperandStack( pVirtualMachineState );
  int integer1 = GetIntegerFromOperandStack( pVirtualMachineState );

  pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( integer1 / integer2 ) ) );
}

void BasicExecutionEngine::ExecuteOpCodePushLong( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, int64_t value )
{
  pVirtualMachineState->PushOperand( new JavaLong( JavaLong::FromHostInt64( value ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeLoadLongFromLocalWithIndex( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  ExecuteOpCodeLoadLongFromLocal( pVirtualMachineState, ReadByteUnsigned( pVirtualMachineState ) );
}

void BasicExecutionEngine::ExecuteOpCodeLoadLongFromLocal( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, uint8_t localVariableIndex )
{
#if defined (_DEBUG) && defined (JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        pVirtualMachineState->LogLocalVariables();
    }
#endif // _DEBUG) && defined (JVMX_LOG_VERBOSE)

  boost::intrusive_ptr<IJavaVariableType> pLocalVariable = pVirtualMachineState->GetLocalVariable( localVariableIndex );
  boost::intrusive_ptr<JavaLong> pLongValue = nullptr;

  if ( e_JavaVariableTypes::Long == pLocalVariable->GetVariableType() )
  {
    pLongValue = boost::dynamic_pointer_cast<JavaLong>( pVirtualMachineState->GetLocalVariable( localVariableIndex ) );
  }
  else if ( pLocalVariable->IsIntegerCompatible() )
  {
    boost::intrusive_ptr<JavaInteger> pIntValue = TypeParser::UpCastToInteger( pLocalVariable.get() );
    pLongValue = new JavaLong( JavaLong::FromHostInt64( pIntValue->ToHostInt32() ) );
  }
  else
  {
    throw InvalidStateException( __FUNCTION__ " - Expected long in local variable." );
  }

#if defined(_DEBUG) && defined (JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      GetLogger()->LogDebug("Loading Long %s from local variable %d", pLongValue->ToString().ToUtf8String().c_str(), localVariableIndex);
  }
#endif // _DEBUG
  pVirtualMachineState->PushOperand( pLongValue );
}

void BasicExecutionEngine::ExecuteOpCodeLoadByteOrBooleanFromArray( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  if ( e_JavaVariableTypes::Integer != pVirtualMachineState->PeekOperand()->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected integer on the operand stack." );
  }

  boost::intrusive_ptr< JavaInteger > pIndex = boost::dynamic_pointer_cast<JavaInteger>( pVirtualMachineState->PopOperand() );
  if ( e_JavaVariableTypes::Array != pVirtualMachineState->PeekOperand()->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected Array on the operand stack." );
  }

  boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pArray )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected reference to JavaArray on operand stack." );
  }

  if ( pArray->IsNull() )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaNullPointerExceptionException );
    return;
  }

  if ( pIndex->ToHostInt32() < 0 || pIndex->ToHostInt32() > static_cast<int32_t>( pArray->GetContainedArray()->GetNumberOfElements() ) )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaArrayIndexOutOfBoundsException );
    return;
  }

  if ( pArray->GetContainedArray()->GetContainedType() != e_JavaArrayTypes::Boolean && pArray->GetContainedArray()->GetContainedType() != e_JavaArrayTypes::Byte )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected boolean or byte in the array." );
  }

  IJavaVariableType *pArrayValueAtIndex = pArray->GetContainedArray()->At( pIndex->ToHostInt32() );
  JVMX_ASSERT( pArrayValueAtIndex->IsIntegerCompatible() );

  if ( e_JavaVariableTypes::Bool == pArrayValueAtIndex->GetVariableType() )
  {
    JavaBool *pValueAsBool = dynamic_cast<JavaBool *>( pArrayValueAtIndex );
    boost::intrusive_ptr<JavaInteger> valueAsInt = new JavaInteger( JavaInteger::FromHostInt32( pValueAsBool->ToBool() ? 1 : 0 ) );

    pVirtualMachineState->PushOperand( valueAsInt );
  }
  else
  {
    JavaByte *pValueAsByte = dynamic_cast<JavaByte *>( pArrayValueAtIndex );
    boost::intrusive_ptr<JavaInteger> valueAsInt = new JavaInteger( JavaInteger::FromHostInt32( static_cast<int32_t>( pValueAsByte->ToHostInt8() ) ) );

    pVirtualMachineState->PushOperand( valueAsInt );
  }
}

void BasicExecutionEngine::ExecuteOpCodeXORInteger( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        pVirtualMachineState->LogOperandStack();
    }
#endif // _DEBUG

  int integer2 = GetIntegerFromOperandStack( pVirtualMachineState );
  int integer1 = GetIntegerFromOperandStack( pVirtualMachineState );

  pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( integer1 ^ integer2 ) ) );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeThrowReference( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        pVirtualMachineState->LogOperandStack();
    }
#endif // _DEBUG

  auto pOperand = pVirtualMachineState->PopOperand();
  if ( !pOperand->IsReferenceType() )
  {
    throw InvalidStateException( __FUNCTION__  " - Expected a reference type when throwing exception." );
  }

  if ( pOperand->IsNull() )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaNullPointerExceptionException );
    return;
  }

  boost::intrusive_ptr<ObjectReference> pObject = boost::dynamic_pointer_cast<ObjectReference>( pOperand );
  if ( nullptr == pObject )
  {
    throw NotImplementedException( __FUNCTION__ " - Arrays not yet implemented." );
  }

  if ( !IsInstanceOf( pVirtualMachineState, new JavaString( c_ThrowableClassName ), pObject->GetContainedObject()->GetClass()->GetName() ) )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected class to implement Throwable." );
  }

  HelperClasses::ThrowJavaExceptionInternal( pVirtualMachineState.get(), pObject );
}

int16_t BasicExecutionEngine::ReadShort( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  if ( !pVirtualMachineState->CanReadBytes( sizeof( ConstantPoolIndex ) ) )
  {
    throw InvalidStateException( __FUNCTION__ " - Not enough space in current code segment to read a short." );
  }

  int8_t branchByte1 = pVirtualMachineState->GetCodeSegmentStart()[ pVirtualMachineState->GetProgramCounter() ];
  pVirtualMachineState->AdvanceProgramCounter( 1 );
  uint8_t branchByte2 = pVirtualMachineState->GetCodeSegmentStart()[ pVirtualMachineState->GetProgramCounter() ];
  pVirtualMachineState->AdvanceProgramCounter( 1 );

  return ( branchByte1 << 8 ) | branchByte2;
}

void BasicExecutionEngine::ValidateStaticFieldType( std::shared_ptr<ConstantPoolFieldReference> pFieldEntry, std::shared_ptr<FieldInfo> pFieldInfo, boost::intrusive_ptr<IJavaVariableType> pOperand )
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    GetLogger()->LogDebug("Field %s::%s descriptor is: %s", pFieldEntry->GetClassName()->ToUtf8String().c_str(), pFieldInfo->GetName()->ToUtf8String().c_str(), pFieldInfo->GetType()->ToUtf8String().c_str());
#endif // _DEBUG
  switch ( pFieldInfo->GetType()->At( 0 ) )
  {
    case c_JavaTypeSpecifierByte:
    case c_JavaTypeSpecifierChar:
    case c_JavaTypeSpecifierInteger:
    case c_JavaTypeSpecifierShort:
    case c_JavaTypeSpecifierBool:
      {
        boost::intrusive_ptr< JavaInteger > pInteger = boost::dynamic_pointer_cast<JavaInteger>( pOperand );
        if ( nullptr == pInteger )
        {
          throw InvalidArgumentException( __FUNCTION__ " - Expected operand to be integer type." );
        }
      }
      break;

    case c_JavaTypeSpecifierFloat:
      {
        boost::intrusive_ptr<JavaFloat> pFloat = boost::dynamic_pointer_cast<JavaFloat>( pOperand );
        if ( nullptr == pFloat )
        {
          throw InvalidArgumentException( __FUNCTION__ " - Expected operand to be float type." );
        }
      }
      break;

    case c_JavaTypeSpecifierDouble:
      {
        boost::intrusive_ptr<JavaDouble> pDouble = boost::dynamic_pointer_cast<JavaDouble>( pOperand );
        if ( nullptr == pDouble )
        {
          throw InvalidArgumentException( __FUNCTION__ " - Expected operand to be double type." );
        }
      }
      break;

    case c_JavaTypeSpecifierLong:
      {
        boost::intrusive_ptr<JavaLong> pLong = boost::dynamic_pointer_cast<JavaLong>( pOperand );
        if ( nullptr == pLong )
        {
          throw InvalidArgumentException( __FUNCTION__ " - Expected operand to be long type." );
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
        if ( pOperand->IsNull() )
        {
          break;
        }

        boost::intrusive_ptr<ObjectReference> pReference = boost::dynamic_pointer_cast<ObjectReference>( pOperand );
        if ( nullptr == pReference )
        {
          throw InvalidArgumentException( __FUNCTION__ " - Expected operand to be reference type." );
        }
      }
      break;

    case c_JavaTypeSpecifierArray:
      {
        boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>( pOperand );
        if ( nullptr == pArray )
        {
          throw InvalidArgumentException( __FUNCTION__ " - Expected operand to be array type." );
        }
      }
      break;

    default:
      throw UnsupportedTypeException( __FUNCTION__ " - Unknown type found." );
      break;
  }
}

void BasicExecutionEngine::ValidateArrayType( uint8_t type )
{
  switch ( static_cast<e_JavaArrayTypes>( type ) )
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
      throw UnsupportedTypeException( __FUNCTION__ " - Creating array of unknown type" );
      break;
  }
}

void BasicExecutionEngine::ExecuteOpCodeConvertIntegerToByte( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  boost::intrusive_ptr< JavaInteger > pInteger1 = boost::dynamic_pointer_cast<JavaInteger>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pInteger1 )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected first operand to be integer type." );
  }

  int32_t value = ConvertIntegerToByte( pInteger1 );

  pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( value ) ) );
}

void BasicExecutionEngine::StoreIntoArray( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, boost::intrusive_ptr< JavaInteger > pValue )
{
  boost::intrusive_ptr< JavaInteger > pIndex = boost::dynamic_pointer_cast<JavaInteger>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pIndex )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected second operand to be integer type." );
  }

  boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pArray )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected first operand to be array type." );
  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      GetLogger()->LogDebug(__FUNCTION__ " - Storing value %s in index %d", (const char*)(pValue->ToString().ToUtf8String().c_str()), pIndex->ToHostInt32());
  }
#endif // _DEBUG

  if ( pArray->IsNull() )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaNullPointerExceptionException );
  }

  if ( pIndex->ToHostInt32() < 0 || static_cast< size_t >( pIndex->ToHostInt32() ) > pArray->GetContainedArray()->GetNumberOfElements() )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaArrayIndexOutOfBoundsException );
  }

  pArray->GetContainedArray()->SetAt( *pIndex, *pValue );
}

int32_t BasicExecutionEngine::ConvertIntegerToByte( boost::intrusive_ptr< JavaInteger > pInteger1 )
{
  int8_t byte = static_cast<int8_t>( pInteger1->ToHostInt32() & 0xFF );
  return byte;
}

bool BasicExecutionEngine::CanExceptionBeHandledLocally( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, intptr_t programCounterBeforeLastInstruction )
{
  return nullptr != FindLocalExceptionTableEntry( pVirtualMachineState, programCounterBeforeLastInstruction );
}

std::shared_ptr<ExceptionTableEntry> BasicExecutionEngine::FindLocalExceptionTableEntry( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, intptr_t programCounterBeforeLastInstruction )
{
  std::shared_ptr< MethodInfo > pMethodInfo = pVirtualMachineState->GetCurrentMethodInfo();
  if ( nullptr == pMethodInfo )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected to get a method info backs." );
  }

  const ClassAttributeCode *pCodeAttributes = pMethodInfo->GetCodeInfo();
  if ( nullptr == pCodeAttributes )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected code attributes to be non-null." );
  }

  const ExceptionTable &exceptionTable = pCodeAttributes->GetExceptionTable();
  for ( auto it : exceptionTable )
  {
    std::shared_ptr<ConstantPoolClassReference> pCatchType = it.GetCatchType();
    if ( nullptr == pCatchType ||
         IsInstanceOf( pVirtualMachineState, pCatchType->GetClassName(), pVirtualMachineState->GetException()->GetContainedObject()->GetClass()->GetName() ) )
    {
      if ( programCounterBeforeLastInstruction >= it.GetStartPosition() &&
           programCounterBeforeLastInstruction < it.GetEndPosition() )
      {
        return std::make_shared<ExceptionTableEntry>( it );
      }
    }
  }

  return nullptr;
}

void BasicExecutionEngine::CatchException( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, intptr_t programCounterBeforeLastInstruction )
{
  std::shared_ptr<ExceptionTableEntry> pEntry = FindLocalExceptionTableEntry( pVirtualMachineState, programCounterBeforeLastInstruction );
  if ( nullptr == pEntry )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected exception table entry to be valid." );
  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      GetLogger()->LogDebug("Exception caught in %s (%s)", pVirtualMachineState->GetCurrentClassAndMethodName().ToUtf8String().c_str(), pVirtualMachineState->GetException()->GetContainedObject()->ToString().ToUtf8String().c_str());
  }
#endif // defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)

  int programCounter = pVirtualMachineState->GetProgramCounter();

#ifdef _DEBUG
  if ( programCounter > pEntry->GetHandlerPosition() )
  {
    JVMX_ASSERT( false );
  }
#endif // _DEBUG

  uint16_t adjustment = static_cast<uint16_t>( pEntry->GetHandlerPosition() - programCounter );
  JVMX_ASSERT( adjustment < 0xF000 );

  // the operand stack of the current frame is cleared,
  RewindOperandStack( pVirtualMachineState, pEntry->GetStartPosition() ); // Rewindin must happen first, because we use the current PC.

  // If an exception handler that matches objectref is found, it contains
  // the location of the code intended to handle this exception.The pc
  // register is reset to that location,
  pVirtualMachineState->AdvanceProgramCounter( adjustment );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      GetLogger()->LogDebug("Exception caught in %s(%s), %s", pVirtualMachineState->GetCurrentClassAndMethodName().ToUtf8String().c_str(), pVirtualMachineState->GetCurrentMethodType().ToUtf8String().c_str(), pVirtualMachineState->GetException()->GetContainedObject()->ToString().ToUtf8String().c_str());
  }
#endif // defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)

  // objectref is pushed back onto the operand stack,  and execution continues
  pVirtualMachineState->PushOperand( pVirtualMachineState->GetException() );
  pVirtualMachineState->ResetException();
}

void BasicExecutionEngine::RewindOperandStack( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, size_t exceptionStartRegion )
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        GetLogger()->LogDebug("Before rewinding operand stack:");
        pVirtualMachineState->LogOperandStack();
    }
#endif // defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)

  size_t numberOfFieldsToClear = pVirtualMachineState->CalculateNumberOfStackItemsToClear( exceptionStartRegion );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      GetLogger()->LogDebug("Popping %lld objects from the operand stack.", (int64_t)numberOfFieldsToClear);
  }
#endif // defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)

  for ( size_t i = 0; i < numberOfFieldsToClear; ++ i )
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

void BasicExecutionEngine::ExecuteOpCodeLongComparison( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  uint64_t long2 = GetLongFromOperandStack( pVirtualMachineState );
  uint64_t long1 = GetLongFromOperandStack( pVirtualMachineState );

  int32_t result = 0;
  if ( long1 > long2 )
  {
    result = 1;
  }
  else if ( long2 > long1 )
  {
    result = -1;
  }
  else
  {
    result = 0;
  }

  pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( result ) ) );
}

void BasicExecutionEngine::ExecuteOpCodePushDoubleOrLongFromConstantPoolWide( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  std::shared_ptr<ConstantPoolEntry> pEntry = pVirtualMachineState->GetConstantFromCurrentClass( ReadIndex( pVirtualMachineState ) );

  if ( e_ConstantPoolEntryTypeDouble == pEntry->GetType() )
  {
    pVirtualMachineState->PushOperand( pEntry->AsDouble() );
  }
  else if ( e_ConstantPoolEntryTypeLong == pEntry->GetType() )
  {
    pVirtualMachineState->PushOperand( pEntry->AsLong() );
  }
  else
  {
    throw InvalidStateException( __FUNCTION__ " - Expected Long or Double constant at the index." );
  }
}

void BasicExecutionEngine::ExecuteOpCodeJumpSubRoutine( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int16_t offset = ReadOffset( pVirtualMachineState );

  boost::intrusive_ptr<JavaReturnAddress> pAddress = new JavaReturnAddress( pVirtualMachineState->GetProgramCounter() );
  pVirtualMachineState->PushOperand( pAddress );

  AdjustProgramCounterByOffset( pVirtualMachineState, offset );
}

void BasicExecutionEngine::ExecuteOpCodeReturnFromSubRoutine( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  //auto programCounter = pVirtualMachineState->GetProgramCounter();
  uint8_t localVariableIndex = ReadByteUnsigned( pVirtualMachineState ); // This will move the PC, so we have to take the PC before this for our calculation.

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      pVirtualMachineState->LogLocalVariables();
      GetLogger()->LogDebug("Loading return address: %s from local variable %d", pVirtualMachineState->GetLocalVariable(localVariableIndex)->ToString().ToUtf8String().c_str(), static_cast<int>(localVariableIndex));
  }
#endif

  boost::intrusive_ptr<IJavaVariableType> pLocalVariable = pVirtualMachineState->GetLocalVariable( localVariableIndex );
  if ( e_JavaVariableTypes::ReturnAddress != pLocalVariable->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected return address as local variable." );
  }

  boost::intrusive_ptr<JavaReturnAddress> pAddress = boost::dynamic_pointer_cast<JavaReturnAddress>( pLocalVariable );

  int16_t offset = static_cast<uint16_t>( pAddress->GetAddress() - pVirtualMachineState->GetProgramCounter() );
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

void BasicExecutionEngine::ExecuteOpCodeReturnLong( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#ifdef _DEBUG
  const size_t debugSize = pVirtualMachineState->GetOperandStackSize();
#endif // _DEBUG

  std::shared_ptr<MethodInfo> pMethodInfo = pVirtualMachineState->GetCurrentMethodInfo();
  if ( pMethodInfo->IsSynchronised() )
  {
    pVirtualMachineState->PopMonitor()->Unlock( pMethodInfo->GetFullName().ToUtf8String().c_str() );
  }

  if ( pVirtualMachineState->HasExceptionOccurred() )
  {
    return;
  }

  boost::intrusive_ptr<IJavaVariableType> pReference = pVirtualMachineState->PopOperand();
  if ( pReference->GetVariableType() != e_JavaVariableTypes::Long )
  {
    throw InvalidStateException( __FUNCTION__ " - Value on the operand stack was not of type long." );
  }

  pVirtualMachineState->PopState();
  pVirtualMachineState->ReleaseLocalVariables();

  pVirtualMachineState->PushOperand( pReference );

#ifdef _DEBUG
  JVMX_ASSERT( pVirtualMachineState->GetOperandStackSize() == debugSize );
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeXORLong( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        pVirtualMachineState->LogOperandStack();
    }
#endif // _DEBUG

  int64_t long2 = GetLongFromOperandStack( pVirtualMachineState );
  int64_t long1 = GetLongFromOperandStack( pVirtualMachineState );

  pVirtualMachineState->PushOperand( new JavaLong( JavaLong::FromHostInt64( long1 ^ long2 ) ) );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeANDLong( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        pVirtualMachineState->LogOperandStack();
    }
#endif // _DEBUG

  int64_t long2 = GetLongFromOperandStack( pVirtualMachineState );
  int64_t long1 = GetLongFromOperandStack( pVirtualMachineState );

  pVirtualMachineState->PushOperand( new JavaLong( JavaLong::FromHostInt64( long1 & long2 ) ) );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeLongMultiply( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        pVirtualMachineState->LogOperandStack();
    }
#endif // _DEBUG

  int64_t long2 = GetLongFromOperandStack( pVirtualMachineState );
  int64_t long1 = GetLongFromOperandStack( pVirtualMachineState );

  pVirtualMachineState->PushOperand( new JavaLong( JavaLong::FromHostInt64( long1 * long2 ) ) );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeLongAdd( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        pVirtualMachineState->LogOperandStack();
    }
#endif // _DEBUG

  int64_t long2 = GetLongFromOperandStack( pVirtualMachineState );
  int64_t long1 = GetLongFromOperandStack( pVirtualMachineState );

  pVirtualMachineState->PushOperand( new JavaLong( JavaLong::FromHostInt64( long1 + long2 ) ) );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeShiftLongRight( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int32_t value2 = GetIntegerFromOperandStack( pVirtualMachineState );
  int64_t value1 = GetLongFromOperandStack( pVirtualMachineState );

  pVirtualMachineState->PushOperand( new JavaLong( JavaLong::FromHostInt64( value1 >> ( value2 & 0x1F ) ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeShiftLongLeft( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int32_t value2 = GetIntegerFromOperandStack( pVirtualMachineState );
  int64_t value1 = GetLongFromOperandStack( pVirtualMachineState );

  pVirtualMachineState->PushOperand( new JavaLong( JavaLong::FromHostInt64( value1 << ( value2 & 0x1F ) ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeConvertLongToInteger( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int64_t value1 = GetLongFromOperandStack( pVirtualMachineState );

  pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( value1 & 0xFFFFFFFF ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeConvertIntegerToLong( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int32_t value1 = GetIntegerFromOperandStack( pVirtualMachineState );

  pVirtualMachineState->PushOperand( new JavaLong( JavaLong::FromHostInt64( value1 & 0xFFFFFFFF ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeConvertLongToDouble( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int64_t value1 = GetLongFromOperandStack( pVirtualMachineState );

  pVirtualMachineState->PushOperand( new JavaDouble( JavaDouble::FromHostInt64( value1 ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeDoubleDivide( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  boost::intrusive_ptr<IJavaVariableType> pOperand2 = pVirtualMachineState->PopOperand();
  boost::intrusive_ptr<IJavaVariableType> pOperand1 = pVirtualMachineState->PopOperand();

  if ( e_JavaVariableTypes::Double != pOperand2->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected double as second parameter." );
  }

  if ( e_JavaVariableTypes::Double != pOperand1->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected double as first parameter." );
  }

  double value1 = boost::dynamic_pointer_cast<JavaDouble>( pOperand1 )->ToHostDouble();
  double value2 = boost::dynamic_pointer_cast<JavaDouble>( pOperand2 )->ToHostDouble();

  double result = value1 / value2;

  pVirtualMachineState->PushOperand( new JavaDouble( JavaDouble::FromHostDouble( result ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeReturnDouble( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#ifdef _DEBUG
  const size_t debugSize = pVirtualMachineState->GetOperandStackSize();
#endif // _DEBUG

  std::shared_ptr<MethodInfo> pMethodInfo = pVirtualMachineState->GetCurrentMethodInfo();
  if ( pMethodInfo->IsSynchronised() )
  {
    pVirtualMachineState->PopMonitor()->Unlock( pMethodInfo->GetFullName().ToUtf8String().c_str() );
  }

  if ( pVirtualMachineState->HasExceptionOccurred() )
  {
    return;
  }

  boost::intrusive_ptr<IJavaVariableType> pReference = pVirtualMachineState->PopOperand();
  if ( pReference->GetVariableType() != e_JavaVariableTypes::Double )
  {
    throw InvalidStateException( __FUNCTION__ " - Value on the operand stack was not of type double." );
  }

  pVirtualMachineState->PopState();
  pVirtualMachineState->ReleaseLocalVariables();

  pVirtualMachineState->PushOperand( pReference );

#ifdef _DEBUG
  JVMX_ASSERT( pVirtualMachineState->GetOperandStackSize() == debugSize );
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeStoreDoubleInLocal( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, uint8_t localVariableIndex )
{
  if ( e_JavaVariableTypes::Double != pVirtualMachineState->PeekOperand()->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected double on the operand stack." );
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

  pVirtualMachineState->SetLocalVariable( localVariableIndex, pVirtualMachineState->PopOperand() );
  pVirtualMachineState->SetLocalVariable( localVariableIndex + 1, new ObjectReference( nullptr ) );

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      GetLogger()->LogDebug("**After:");
      pVirtualMachineState->LogLocalVariables();
  }
#endif
}

void BasicExecutionEngine::ExecuteOpCodeStoreDoubleInLocalWithIndex( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  ExecuteOpCodeStoreDoubleInLocal( pVirtualMachineState, ReadByteUnsigned( pVirtualMachineState ) );
}

void BasicExecutionEngine::ExecuteOpCodeLoadDoubleFromLocalWithIndex( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  ExecuteOpCodeLoadDoubleFromLocal( pVirtualMachineState, ReadByteUnsigned( pVirtualMachineState ) );
}

void BasicExecutionEngine::ExecuteOpCodeLoadDoubleFromLocal( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, uint8_t localVariableIndex )
{
  if ( e_JavaVariableTypes::Double != pVirtualMachineState->GetLocalVariable( localVariableIndex )->GetVariableType() )
  {
    pVirtualMachineState->LogLocalVariables();
    throw InvalidStateException( __FUNCTION__ " - Expected Double in local variable." );
  }

  pVirtualMachineState->PushOperand( pVirtualMachineState->GetLocalVariable( localVariableIndex ) );
}

void BasicExecutionEngine::ExecuteOpCodeNegateDouble( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  if ( e_JavaVariableTypes::Double != pVirtualMachineState->PeekOperand()->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected double on the operand stack." );
  }

  boost::intrusive_ptr<JavaDouble> pOperand = boost::dynamic_pointer_cast<JavaDouble>( pVirtualMachineState->PopOperand() );

  double negatedValue = ( - pOperand->ToHostDouble() ); // Apply operator -()

  pVirtualMachineState->PushOperand( new JavaDouble( JavaDouble::FromHostDouble( negatedValue ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeANDInteger( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        pVirtualMachineState->LogOperandStack();
    }
#endif // _DEBUG

  int32_t int2 = GetIntegerFromOperandStack( pVirtualMachineState );
  int32_t int1 = GetIntegerFromOperandStack( pVirtualMachineState );

  pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( int1 & int2 ) ) );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeDuplicateTopOperandOrTwo( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        pVirtualMachineState->LogOperandStack();
    }
#endif // _DEBUG

  auto pPeekValue = pVirtualMachineState->PeekOperand();
  if ( IsCategoryTwoType( pPeekValue ) )
  {
    pVirtualMachineState->PushOperand( pPeekValue );
  }
  else
  {
    auto pValue2 = pVirtualMachineState->PeekOperand();
    pVirtualMachineState->PopOperand();

    auto pValue1 = pVirtualMachineState->PeekOperand();
    pVirtualMachineState->PushOperand(pValue2);
    
    pVirtualMachineState->PushOperand( pValue1 );
    pVirtualMachineState->PushOperand(pValue2);
  }

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeLoadIntegerFromArray( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        pVirtualMachineState->LogOperandStack();
    }
#endif

  if ( e_JavaVariableTypes::Integer != pVirtualMachineState->PeekOperand()->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected integer on the operand stack." );
  }

  boost::intrusive_ptr< JavaInteger > pIndex = boost::dynamic_pointer_cast<JavaInteger>( pVirtualMachineState->PopOperand() );

#if 0
  GetLogger()->LogDebug("Before");
  pVirtualMachineState->LogCallStack();
  auto operand = pVirtualMachineState->PeekOperand();
  auto type = operand->GetVariableType();
  GetLogger()->LogDebug("After");
#endif

  if ( e_JavaVariableTypes::Array != pVirtualMachineState->PeekOperand()->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected Array on the operand stack." );
  }

  boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>( pVirtualMachineState->PopOperand() );

  if ( nullptr == pArray )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected reference to JavaArray on operand stack." );
  }

  if ( pArray->IsNull() )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaNullPointerExceptionException );
    return;
  }

  if ( pIndex->ToHostInt32() < 0 || pIndex->ToHostInt32() > static_cast<int32_t>( pArray->GetContainedArray()->GetNumberOfElements() ) )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaArrayIndexOutOfBoundsException );
    return;
  }

  JVMX_ASSERT( pArray->GetContainedArray()->GetContainedType() == e_JavaArrayTypes::Integer );

  const JavaInteger *pResult = dynamic_cast<const JavaInteger *>( pArray->GetContainedArray()->At( pIndex->ToHostInt32() ) );
  if ( nullptr == pResult )
  {
    throw InvalidStateException( __FUNCTION__ " - Could not convert from array contained type to integer." );
  }

  pVirtualMachineState->PushOperand( new JavaInteger( *pResult ) );
}

void BasicExecutionEngine::ExecuteOpCodeReturnFloat( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#ifdef _DEBUG
  const size_t debugSize = pVirtualMachineState->GetOperandStackSize();
#endif // _DEBUG

  std::shared_ptr<MethodInfo> pMethodInfo = pVirtualMachineState->GetCurrentMethodInfo();
  if ( pMethodInfo->IsSynchronised() )
  {
    pVirtualMachineState->PopMonitor()->Unlock( pMethodInfo->GetFullName().ToUtf8String().c_str() );
  }

  if ( pVirtualMachineState->HasExceptionOccurred() )
  {
    return;
  }

  boost::intrusive_ptr<IJavaVariableType> pReference = pVirtualMachineState->PopOperand();
  if ( pReference->GetVariableType() != e_JavaVariableTypes::Float )
  {
    throw InvalidStateException( __FUNCTION__ " - Value on the operand stack was not of type float." );
  }

  pVirtualMachineState->PopState();
  pVirtualMachineState->ReleaseLocalVariables();

  pVirtualMachineState->PushOperand( pReference );

#ifdef _DEBUG
  JVMX_ASSERT( pVirtualMachineState->GetOperandStackSize() == debugSize );
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeLongSubtract( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int64_t integer2 = GetLongFromOperandStack( pVirtualMachineState );
  int64_t integer1 = GetLongFromOperandStack( pVirtualMachineState );

  pVirtualMachineState->PushOperand( new JavaLong( JavaLong::FromHostInt64( integer1 - integer2 ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeORInteger( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  int32_t integer2 = GetIntegerFromOperandStack( pVirtualMachineState );
  int32_t integer1 = GetIntegerFromOperandStack( pVirtualMachineState );

  pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( integer1 | integer2 ) ) );
}

bool BasicExecutionEngine::IsCategoryTwoType( boost::intrusive_ptr<IJavaVariableType> pValue )
{
  return e_JavaVariableTypes::Long == pValue->GetVariableType() || e_JavaVariableTypes::Double == pValue->GetVariableType();
}

bool BasicExecutionEngine::IsCategoryOneType( boost::intrusive_ptr<IJavaVariableType> pValue )
{
  return e_JavaVariableTypes::Long != pValue->GetVariableType() && e_JavaVariableTypes::Double != pValue->GetVariableType();
}

void BasicExecutionEngine::ExecuteOpCodeConvertDoubleToInt( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  boost::intrusive_ptr<JavaDouble> pDouble = boost::dynamic_pointer_cast<JavaDouble>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pDouble )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected first operand to be float type." );
  }

  boost::intrusive_ptr< JavaInteger > result = new JavaInteger( JavaInteger::FromDefault() );
  if ( pDouble->IsNan() )
  {
    result = new JavaInteger( JavaInteger::FromHostInt32( 0 ) );
  }
  else
  {
    result = new JavaInteger( JavaInteger::FromHostInt32( static_cast<int32_t>( pDouble->ToHostDouble() ) ) );
  }

  pVirtualMachineState->PushOperand( result );
}

void BasicExecutionEngine::ExecuteOpCodeFloatDivide( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  boost::intrusive_ptr<IJavaVariableType> pOperand2 = pVirtualMachineState->PopOperand();
  boost::intrusive_ptr<IJavaVariableType> pOperand1 = pVirtualMachineState->PopOperand();

  if ( e_JavaVariableTypes::Float != pOperand2->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected float as second parameter." );
  }

  if ( e_JavaVariableTypes::Float != pOperand1->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected float as first parameter." );
  }

  float value1 = boost::dynamic_pointer_cast<JavaFloat>( pOperand1 )->ToHostFloat();
  float value2 = boost::dynamic_pointer_cast<JavaFloat>( pOperand2 )->ToHostFloat();

  float result = value1 / value2;

  pVirtualMachineState->PushOperand( new JavaFloat( JavaFloat::FromHostFloat( result ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeConvertDoubleToFloat( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  boost::intrusive_ptr< JavaDouble > pDouble1 = boost::dynamic_pointer_cast<JavaDouble>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pDouble1 )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected first operand to be double type." );
  }

  pVirtualMachineState->PushOperand( new JavaFloat( JavaFloat::FromHostDouble( static_cast<double>( pDouble1->ToHostDouble() ) ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeDoubleMultiply( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  boost::intrusive_ptr<JavaDouble> pDouble2 = boost::dynamic_pointer_cast<JavaDouble>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pDouble2 )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected first operand to be float type." );
  }

  boost::intrusive_ptr<JavaDouble> pDouble1 = boost::dynamic_pointer_cast<JavaDouble>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pDouble1 )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected second operand to be float type." );
  }

  pVirtualMachineState->PushOperand( new JavaDouble( JavaDouble::FromHostDouble( pDouble1->ToHostDouble() * pDouble2->ToHostDouble() ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeDoubleAdd( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        pVirtualMachineState->LogOperandStack();
    }
#endif // _DEBUG
  boost::intrusive_ptr<IJavaVariableType> pOperand2 = pVirtualMachineState->PopOperand();
  boost::intrusive_ptr<IJavaVariableType> pOperand1 = pVirtualMachineState->PopOperand();

  if ( e_JavaVariableTypes::Double != pOperand2->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected a float on the operand stack." );
  }

  if ( e_JavaVariableTypes::Double != pOperand1->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected a float on the operand stack." );
  }

  double value2 = boost::dynamic_pointer_cast<JavaDouble>( pOperand2 )->ToHostDouble();
  double value1 = boost::dynamic_pointer_cast<JavaDouble>( pOperand1 )->ToHostDouble();

  pVirtualMachineState->PushOperand( new JavaDouble( JavaDouble::FromHostDouble( value1 + value2 ) ) );

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeConvertFloatToDouble( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  boost::intrusive_ptr< JavaFloat > pFloat1 = boost::dynamic_pointer_cast<JavaFloat>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pFloat1 )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected first operand to be float type." );
  }

  pVirtualMachineState->PushOperand( new JavaDouble( JavaDouble::FromHostDouble( static_cast< double >( pFloat1->ToHostFloat() ) ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeLoadFloatFromArray( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  if ( e_JavaVariableTypes::Integer != pVirtualMachineState->PeekOperand()->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected integer on the operand stack." );
  }

  boost::intrusive_ptr< JavaInteger > pIndex = boost::dynamic_pointer_cast<JavaInteger>( pVirtualMachineState->PopOperand() );

  if ( e_JavaVariableTypes::Array != pVirtualMachineState->PeekOperand()->GetVariableType() )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected Array on the operand stack." );
  }

  boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pArray )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected reference to JavaArray on operand stack." );
  }

  if ( pArray->IsNull() )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaNullPointerExceptionException );
    return;
  }

  if ( pIndex->ToHostInt32() < 0 || pIndex->ToHostInt32() > static_cast<int32_t>( pArray->GetContainedArray()->GetNumberOfElements() ) )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaArrayIndexOutOfBoundsException );
    return;
  }

  JavaFloat *pTempFloat = dynamic_cast<JavaFloat *>( pArray->GetContainedArray()->At( pIndex->ToHostInt32() ) );
  if ( nullptr == pTempFloat )
  {
    throw InvalidStateException( __FUNCTION__ " - Expected float in array." );
  }

  boost::intrusive_ptr<JavaFloat> pFloat = new JavaFloat( JavaFloat::FromHostFloat( pTempFloat->ToHostFloat() ) );
  pVirtualMachineState->PushOperand( pFloat );
}

void BasicExecutionEngine::ExecuteOpCodeStoreIntoFloatArray( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
#if defined _DEBUG && defined(JVMX_LOG_VERBOSE)
    if (pVirtualMachineState->HasUserCodeStarted())
    {
        pVirtualMachineState->LogCallStack();
        pVirtualMachineState->LogOperandStack();
    }
#endif // _DEBUG

  boost::intrusive_ptr<IJavaVariableType> pValue = pVirtualMachineState->PopOperand();
  if ( e_JavaVariableTypes::Float != pValue->GetVariableType() )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected third operand to be float type." );
  }

  boost::intrusive_ptr< JavaInteger > pIndex = boost::dynamic_pointer_cast<JavaInteger>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pIndex )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected second operand to be integer type." );
  }

  boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>( pVirtualMachineState->PopOperand() );
  if ( nullptr == pArray )
  {
    throw InvalidArgumentException( __FUNCTION__ " - Expected first operand to be array type." );
  }

  if ( pArray->IsNull() )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaNullPointerExceptionException );
  }

  if ( pIndex->ToHostInt32() < 0 || static_cast< size_t >( pIndex->ToHostInt32() ) > pArray->GetContainedArray()->GetNumberOfElements() )
  {
    HelperClasses::ThrowJavaException( pVirtualMachineState, c_JavaArrayIndexOutOfBoundsException );
  }

  pArray->GetContainedArray()->SetAt( *pIndex, pValue.get() );
}

void BasicExecutionEngine::ExecuteOpCodeDoubleComparisonL( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( CompareDoubleValues( pVirtualMachineState, -1 ) ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeDoubleComparisonG( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  pVirtualMachineState->PushOperand( new JavaInteger( JavaInteger::FromHostInt32( CompareDoubleValues( pVirtualMachineState, 1 ) ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeDoubleSubtract( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
{
  double double2 = boost::dynamic_pointer_cast<JavaDouble>( pVirtualMachineState->PopOperand() )->ToHostDouble();
  double double1 = boost::dynamic_pointer_cast<JavaDouble>( pVirtualMachineState->PopOperand() )->ToHostDouble();

  pVirtualMachineState->PushOperand( new JavaDouble( JavaDouble::FromHostDouble( double1 - double2 ) ) );
}

void BasicExecutionEngine::ExecuteOpCodeDuplicateTopOperandx2( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState )
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

  if ( IsCategoryOneType( pValue2 ) )
  {
    auto pValue3 = pVirtualMachineState->PopOperand();
    JVMX_ASSERT( IsCategoryOneType( pValue3 ) && IsCategoryOneType( pValue2 ) );

    pVirtualMachineState->PushOperand( pValue1 );
    pVirtualMachineState->PushOperand( pValue3 );
    pVirtualMachineState->PushOperand( pValue2 );
    pVirtualMachineState->PushOperand( pValue1 );
  }
  else
  {
    JVMX_ASSERT( IsCategoryOneType( pValue1 ) && IsCategoryTwoType( pValue2 ) );

    pVirtualMachineState->PushOperand( pValue1 );
    pVirtualMachineState->PushOperand( pValue2 );
    pVirtualMachineState->PushOperand( pValue1 );
  }

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
      GetLogger()->LogDebug("**After:");
      pVirtualMachineState->LogOperandStack();
  }
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

  // TODO: We may have to do some more stuff here to kill the thread!
  // I think we are safe, because in theory only the main thread will ever get here and all that is left is cleanup.
}