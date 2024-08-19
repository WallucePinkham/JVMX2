#ifndef _BASICEXECUTIONENGINE__H_
#define _BASICEXECUTIONENGINE__H_

#include <map>
#include <wallaroo/collaborator.h>

class ExceptionTableEntry;
class StackFrame;

#include "ImmediateReturnRequired.h"
#include "IncreaseCallStackDepth.h"
#include "MethodAlreadyIdentified.h"

#include "IExecutionEngine.h"

class BasicExecutionEngine : public IExecutionEngine
{
public:
  BasicExecutionEngine();

  virtual ~BasicExecutionEngine();

  virtual void Run( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState ) JVMX_OVERRIDE;

  e_ImmediateReturnRequired ProcessNextOpcode( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, std::shared_ptr<ILogger> pLogger );

  virtual void ThrowJavaException( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, const JVMX_ANSI_CHAR_TYPE *javaExceptionName ) JVMX_OVERRIDE;
  virtual void ThrowJavaException( IVirtualMachineState *pVirtualMachineState, const JVMX_ANSI_CHAR_TYPE *javaExceptionName ) JVMX_OVERRIDE;

  virtual void ThrowJavaExceptionInternal( IVirtualMachineState *pVirtualMachineState, boost::intrusive_ptr<ObjectReference> pException );

  virtual std::shared_ptr<MethodInfo> IdentifyVirtualMethodToCall( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, std::shared_ptr<MethodInfo> pMethodInfo, boost::intrusive_ptr<ObjectReference> pObject ) JVMX_OVERRIDE;

private:
  uint16_t GetNextInstruction( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );

  ConstantPoolIndex ReadIndex( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  int16_t ReadOffset( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState ); // Note that ReadOffset is signed.
  //uint8_t ReadByte( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  uint8_t ReadByteUnsigned( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  int8_t ReadByteSigned( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  int16_t ReadShort( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  //int8_t ReadSignedByte( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  int32_t Read32BitOffset( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );

  std::shared_ptr<FieldInfo> ResolveField( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, const JavaString &className, const JavaString &fieldName );

  std::shared_ptr<JavaClass> ResolveClass( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, const JavaString &className );

  //std::shared_ptr<JavaClassFile> LoadClass( JavaString className );

  // ClassName is returned
  std::shared_ptr<MethodInfo> ResolveMethodReference( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, uint16_t methodIndex, JavaString &className );
  //std::shared_ptr<MethodInfo> ResolveMethodOnClass( boost::intrusive_ptr<JavaString> pClassName, const ConstantPoolMethodReference * pMethodRef );
  std::shared_ptr<JavaClass> ResolveClassFromIndex( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, ConstantPoolIndex index );

  bool IsSuperClassOfCurrentClass( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, boost::intrusive_ptr<JavaString> pClassName ) const;
  bool IsSuperClassOf( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, boost::intrusive_ptr<JavaString> pPossibleSuperClassName, boost::intrusive_ptr<JavaString> pDerivedClassName ) const;
  bool IsInstanceOf( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, boost::intrusive_ptr<JavaString> pPossibleSuperClassName, boost::intrusive_ptr<JavaString> pDerivedClassName ) const;

  static const char *TranslateOpCode( uint16_t opcode );

  int GetIntegerFromOperandStack( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState ) const;
  uint64_t GetLongFromOperandStack( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );

  static bool IsReference( e_JavaVariableTypes type );

  void AdjustProgramCounterByOffset( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, int16_t offset );

  int CompareFloatValues( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, int nanResult );
  int CompareDoubleValues( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, int nanResult );

  bool AreTypesCompatibile( boost::intrusive_ptr<JavaString> referenceType, boost::intrusive_ptr<JavaString> valueType );

  bool DoesClassImplementInterface( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, std::shared_ptr<JavaClass> pClass, boost::intrusive_ptr<JavaString> nameOfInterface );

  e_IncreaseCallStackDepth ExecuteVirtualMethod( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, std::shared_ptr<ConstantPoolEntry> pConstant );

  void ExecuteVirtualMethodForArray( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, std::vector<boost::intrusive_ptr<IJavaVariableType> > paramArray, std::shared_ptr<MethodInfo> pMethodInfo );

  e_IncreaseCallStackDepth ExecuteVirtualMethodForObject( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, std::vector<boost::intrusive_ptr<IJavaVariableType> > paramArray, std::shared_ptr<MethodInfo> pMethodInfo );

  e_IncreaseCallStackDepth ExecuteVirtualMethodInternal( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, std::shared_ptr<MethodInfo> &pMethodInfo, boost::intrusive_ptr<ObjectReference> pObject, std::vector<boost::intrusive_ptr<IJavaVariableType> > paramArray, e_MethodAlreadyIdentified methodAlreadyIdentified );

  void CheckCastForArrays( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, boost::intrusive_ptr<IJavaVariableType> pOperand, std::shared_ptr<JavaClass> pResolvedClass, bool isClassRefArray, boost::intrusive_ptr<JavaString> className );
  void CheckCastForObjects( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, boost::intrusive_ptr<IJavaVariableType> pOperand, std::shared_ptr<JavaClass> pResolvedClass );

  void Halt() JVMX_OVERRIDE;

  std::shared_ptr<IClassLibrary> GetClassLibrary() const;
  std::shared_ptr<ILogger> GetLogger() const;

  void TryDoGarbageCollection( const std::shared_ptr<IVirtualMachineState> &pVirtualMachineState, const std::shared_ptr<IGarbageCollector> &pGarbageCollector );

  //boost::intrusive_ptr<ObjectReference> CreateMetodTypeFromMethodReference( std::shared_ptr<ConstantPoolMethodReference> pMethodReference );

#ifdef _DEBUG
  void BreakDebug( const uint8_t *String1, const uint8_t *String2 );
  void BreakDebug( const uint8_t *String1, const char *String2 );
  void BreakDebug( const char *String1, const char *String2 );
  void BreakDebug( const char16_t *String1, const char16_t *String2 );
#endif // _DEBUG

  void ValidateStaticFieldType( std::shared_ptr<ConstantPoolFieldReference> pFieldEntry, std::shared_ptr<FieldInfo> pFieldInfo, boost::intrusive_ptr<IJavaVariableType> pOperand );
  void ValidateArrayType( uint8_t type );

  void StoreIntoArray( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, boost::intrusive_ptr< JavaInteger > pValue );

  int32_t ConvertIntegerToByte( boost::intrusive_ptr< JavaInteger > pIntegerValue );
  bool CanExceptionBeHandledLocally( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, intptr_t programCounterBeforeLastInstruction );

  std::shared_ptr<ExceptionTableEntry> FindLocalExceptionTableEntry( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, intptr_t programCounterBeforeLastInstruction );

  void CatchException( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, intptr_t programCounterBeforeLastInstruction );
  void RewindOperandStack( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, size_t exceptionStartRegion );

  bool IsCategoryTwoType( boost::intrusive_ptr<IJavaVariableType> pValue );
  bool IsCategoryOneType( boost::intrusive_ptr<IJavaVariableType> pValue );

  void InitialiseDimention( const std::vector<int32_t> &dimentionSizes, const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, boost::intrusive_ptr<ObjectReference> pFirstDimention, uint8_t dimentionCount, int32_t currentDimention, e_JavaArrayTypes finalDimentionType );

  void HandleUnhandledException(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState);

  // Op Codes
private:
  void ExecuteOpCodeGetStatic( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeLoadReferenceFromLocalWithSpecifiedIndex( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeLoadReferenceFromLocalIndex( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, uint8_t index );
  void ExecuteOpCodeReturnVoid( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  e_IncreaseCallStackDepth ExecuteOpCodeInvokeStatic( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeMonitorEnter( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeMonitorExit( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodePushNull( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeNewMultiDimentionalArray( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );

  void ExecuteOpCodePutStatic( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );

  void ExecuteOpCodeLoadReferenceFromConstantPool( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  e_IncreaseCallStackDepth ExecuteOpCodeInvokeVirtual( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );

  void ExecuteOpCodePushInt( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, int value );
  void ExecuteOpCodeNewArrayOfReference( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeNew( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );

  void ExecuteOpCodeDuplicateTopOperand( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeDuplicateTopOperandx1( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  e_IncreaseCallStackDepth ExecuteOpCodeInvokeSpecial( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeStoreLongInLocalWithIndex( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeStoreLongInLocal( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, uint8_t localVariableIndex );
  void ExecuteOpCodePushIntImmediateByte( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeNewArray( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );

  void ExecuteOpCodePushFromConstantPoolWide( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeStoreIntoIntArray( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeGetField( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecutedOpBranchIfEquals( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );

  void ExecutedOpCodeBranchIfNotEquals( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecutedOpCodeBranchIfLessThan( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecutedOpCodeBranchIfGreaterThan( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecutedOpCodeBranchIfGreaterThanOrEqual( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecutedOpCodeBranchIfLessThanOrEqual( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeArrayLength( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );

  void ExecutedOpBranchIfIntegerEquals( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecutedOpBranchIfIntegerNotEquals( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecutedOpBranchIfIntegerLessThan( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecutedOpBranchIfIntegerGreatherThanOrEqual( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecutedOpBranchIfIntegerGreatherThan( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecutedOpBranchIfIntegerLessThanOrEqual( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeBranchIfNotNull( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeBranchIfNull( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );

  void ExecuteOpCodeStoreReferenceInLocalWithIndex( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeStoreReferenceInLocal( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, uint8_t localVariableIndex );
  void ExecuteOpCodeGoto( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeReturnReference( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeReturnInteger( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeIntegerRemainder( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeStoreIntegerInLocal( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, uint8_t localVariableIndex );
  void ExecuteOpCodeStoreIntegerInLocalWithIndex( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );

  void ExecuteOpCodeLoadIntegerFromLocalWithIndex( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeLoadIntegerFromLocal( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, uint8_t localVariableIndex );
  void ExecuteOpCodeLoadReferenceFromArray( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeLoadFloatFromLocalWithIndex( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpLoadFloatFromLocal( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, uint8_t localVariableIndex );
  void ExecuteOpCodePushFloatConstant( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, float value );
  void ExecuteOpCodeFloatComparisonL( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeFloatComparisonG( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodePutField( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeIntegerAdd( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeIntegerMultiply( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeIntegerSubtract( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeStoreIntoCharArray( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeStoreIntoByteArray( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );

  void ExecuteOpCodeStoreIntoReferenceArray( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeTableSwitch( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeConvertIntegerToFloat( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeConvertIntegerToDouble( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeFloatMultiply( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeConvertFloatToInteger( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeIsInstanceOf( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeCheckCast( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeConvertIntegerToByte( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );

  void ExecuteOpCodePopOperandStack( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeBranchIfReferencesAreEqual( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeBranchIfReferencesAreNotEqual( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeIncrementLocalVariable( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeLoadCharacterFromArray( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeNegateInteger( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeConvertIntegerToChar( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeShiftIntegerLeft( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeShiftIntegerRightArithmetic( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeShiftIntegerRightLogical( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  e_IncreaseCallStackDepth ExecuteOpCodeInvokeInterfaceMethod( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodePushShortConstant( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeIntegerDivide( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodePushLong( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, int64_t value );
  void ExecuteOpCodeLoadLongFromLocalWithIndex( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeLoadLongFromLocal( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, uint8_t localVariableIndex );
  void ExecuteOpCodeLoadByteOrBooleanFromArray( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeXORInteger( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeThrowReference( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeLongComparison( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodePushDoubleOrLongFromConstantPoolWide( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeJumpSubRoutine( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeReturnFromSubRoutine( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeReturnLong( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeXORLong( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeANDLong( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeLongMultiply( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeLongAdd( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeShiftLongRight( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeShiftLongLeft( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeConvertLongToInteger( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeConvertIntegerToLong( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeConvertLongToDouble( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeDoubleDivide( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeReturnDouble( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeStoreDoubleInLocal( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, uint8_t localVariableIndex );
  void ExecuteOpCodeStoreDoubleInLocalWithIndex( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeLoadDoubleFromLocalWithIndex( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeLoadDoubleFromLocal( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, uint8_t localVariableIndex );
  void ExecuteOpCodeNegateDouble( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeANDInteger( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeDuplicateTopOperandOrTwo( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeLoadIntegerFromArray( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeReturnFloat( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeLongSubtract( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeORInteger( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeStoreIntoFloatArray( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeLoadFloatFromArray( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeConvertFloatToDouble( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeDoubleAdd( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeDoubleMultiply( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeConvertDoubleToFloat( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeFloatDivide( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeConvertDoubleToInt( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeStoreFloatInLocalWithIndex( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeStoreFloatInLocal( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, uint8_t localVariableIndex );
  void ExecuteOpCodeFloatSubtract( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodePushDouble( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, double value );
  void ExecuteOpCodeDoubleComparisonL( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeDoubleComparisonG( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeFloatAdd( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeDoubleSubtract( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeDuplicateTopOperandx2( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeConvertIntegerToShort( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeDuplicateTopOperandOrTwox1( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeLookupSwitch( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeStoreIntoDoubleArray( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeLoadDoubleFromArray( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );
  void ExecuteOpCodeORLong( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState );

  private:
#ifdef _DEBUG
  int64_t m_InstructionsExecuted;
#endif // _DEBUG

  // TODO: Consider moving this to the constant pool
  std::map< std::pair< JavaString, JavaString>, std::shared_ptr<FieldInfo> > m_ResolvedFields;

  volatile bool m_Halted;
  // TODO: Cache References so they only have to be resolved once.
 
};

#endif // _BASICEXECUTIONENGINE__H_
