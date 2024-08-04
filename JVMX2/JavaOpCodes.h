#ifndef _JAVAOPCODES__H_
#define _JAVAOPCODES__H_

#include "GlobalConstants.h"

enum class e_JavaOpCodes : uint16_t
{
  NoOperation = 0x00
  , PushNull = 0x01
  , PushInt_Minus1 = 0x02
  , PushInt_0 = 0x03
  , PushInt_1 = 0x04
  , PushInt_2 = 0x05
  , PushInt_3 = 0x06
  , PushInt_4 = 0x07
  , PushInt_5 = 0x08
  , PushLong_0 = 0x09
  , PushLong_1 = 0x0a

  , PushDouble_0 = 0x0e
  , PushDouble_1 = 0x0f

  , PushInt_ImmediateByte = 0x10
  , LoadReferenceFromConstantPool = 0x12 //ldc
  , PushFloatConstant_0 = 0x0b
  , PushFloatConstant_1 = 0x0c
  , PushFloatConstant_2 = 0x0d
  , PushShortConstant = 0x11
  , PushFromConstantPoolWide = 0x13
  , PushDoubleOrLongFromConstantPoolWide = 0x14
  , LoadIntegerFromLocal = 0x15
  , LoadLongFromLocal = 0x16
  , LoadFloatFromLocal = 0x17
  , LoadDoubleFromLocal = 0x18
  , LoadReferenceFromLocal = 0x19 //aload

  , LoadIntegerFromLocal_0 = 0x1a
  , LoadIntegerFromLocal_1 = 0x1b
  , LoadIntegerFromLocal_2 = 0x1c
  , LoadIntegerFromLocal_3 = 0x1d

  , LoadLongFromLocal_0 = 0x1e
  , LoadLongFromLocal_1 = 0x1f
  , LoadLongFromLocal_2 = 0x20
  , LoadLongFromLocal_3 = 0x21

  , LoadFloatFromLocal_0 = 0x22
  , LoadFloatFromLocal_1 = 0x23
  , LoadFloatFromLocal_2 = 0x24
  , LoadFloatFromLocal_3 = 0x25

  , LoadDoubleFromLocal_0 = 0x26
  , LoadDoubleFromLocal_1 = 0x27
  , LoadDoubleFromLocal_2 = 0x28
  , LoadDoubleFromLocal_3 = 0x29

  , LoadReferenceFromLocal_0 = 0x2a
  , LoadReferenceFromLocal_1 = 0x2b
  , LoadReferenceFromLocal_2 = 0x2c
  , LoadReferenceFromLocal_3 = 0x2d

  , LoadIntegerFromArray = 0x2e

  , LoadFloatFromArray = 0x30
  , LoadDoubleFromArray = 0x31
  , LoadReferenceFromArray = 0x32
  , LoadByteOrBooleanFromArray = 0x33
  , LoadCharacterFromArray = 0x34
  , StoreLongInLocal = 0x37 //lstore
  , StoreFloatInLocal = 0x38
  , StoreDoubleInLocal = 0x39
  , StoreReferenceInLocal = 0x3a //astore
  , StoreIntegerInLocal = 0x36 //istore
  , StoreIntegerInLocal_0 = 0x3b //istore_0
  , StoreIntegerInLocal_1 = 0x3c //istore_1
  , StoreIntegerInLocal_2 = 0x3d //istore_2
  , StoreIntegerInLocal_3 = 0x3e //istore_3
  , StoreLongInLocal_0 = 0x3f
  , StoreLongInLocal_1 = 0x40
  , StoreLongInLocal_2 = 0x41
  , StoreLongInLocal_3 = 0x42
  , StoreFloatInLocal_0 = 0x43
  , StoreFloatInLocal_1 = 0x44
  , StoreFloatInLocal_2 = 0x45
  , StoreFloatInLocal_3 = 0x46
  , StoreDoubleInLocal_0 = 0x47
  , StoreDoubleInLocal_1 = 0x48
  , StoreDoubleInLocal_2 = 0x49
  , StoreDoubleInLocal_3 = 0x4a
  , StoreReferenceInLocal_0 = 0x4b
  , StoreReferenceInLocal_1 = 0x4c
  , StoreReferenceInLocal_2 = 0x4d
  , StoreReferenceInLocal_3 = 0x4e
  , StoreIntoIntArray = 0x4f

  , StoreIntoFloatArray = 0x51
  , StoreIntoDoubleArray = 0x52
  , StoreIntoReferenceArray = 0x53
  , StoreIntoByteArray = 0x54
  , StoreIntoCharArray = 0x55
  , PopOperandStack = 0x57
  , DuplicateTopOperand = 0x59
  , DuplicateTopOperandx1 = 0x5a
  , DuplicateTopOperandx2 = 0x5b
  , DuplicateTopOperandOrTwo = 0x5c
  , DuplicateTopOperandOrTwox1 = 0x5d

  , IntegerAdd = 0x60
  , LongAdd = 0x61
  , FloatAdd = 0x62
  , DoubleAdd = 0x63
  , IntegerSubtract = 0x64
  , LongSubtract = 0x65
  , FloatSubtract = 0x66
  , DoubleSubtract = 0x67
  , IntegerMultiply = 0x68
  , LongMultiply = 0x69
  , FloatMultiply = 0x6a
  , DoubleMultiply = 0x6b
  , IntegerDivide = 0x6c

  , FloatDivide = 0x6e
  , DoubleDivide = 0x6f
  , IntegerRemainder = 0x70
  , NegateInteger = 0x74
  , NegateDouble = 0x77
  , ShiftIntegerLeft = 0x78
  , ShiftLongLeft = 0x79
  , ShiftIntegerRightArithmetic = 0x7a

  , ShiftIntegerRightLogical = 0x7c
  , ShiftLongRight = 0x7d

  , IntegerAND = 0x7e
  , LongAND = 0x7f

  , IntegerOR = 0x80
  , LongOr = 0x81
  , IntegerXOR = 0x82
  , LongXOR = 0x83
  , IncrementLocalVariable = 0x84
  , ConvertIntegerToLong = 0x85
  , ConvertIntegerToFloat = 0x86
  , ConvertIntegerToDouble = 0x87
  , ConvertLongToInteger = 0x88
  , ConvertLongToDouble = 0x8a
  , ConvertFloatToInteger = 0x8b

  , ConvertFloatToDouble = 0x8d
  , ConvertDoubleToInt = 0x8e

  , ConvertDoubleToFloat = 0x90
  , ConvertIntegerToByte = 0x91
  , ConvertIntegerToChar = 0x92
  , ConvertIntegerToShort = 0x93
  , LongComparison = 0x94
  , FloatingPointComparisonL = 0x95
  , FloatingPointComparisonG = 0x96
  , DoubleComparisonL = 0x97
  , DoubleComparisonG = 0x98
  , BranchIfEquals = 0x99
  , BranchIfNotEquals = 0x9a
  , BranchIfLessThan = 0x9b
  , BranchIfGreaterThanEquals = 0x9c
  , BranchIfGreaterThan = 0x9d
  , BranchIfLessThanEquals = 0x9e
  , BranchIfIntegerEquals = 0x9f
  , BranchIfIntegerNotEquals = 0xa0
  , BranchIfIntegerLessThan = 0xa1
  , BranchIfIntegerGreaterThanEquals = 0xa2
  , BranchIfIntegerGreaterThan = 0xa3
  , BranchIfIntegerLessThanEquals = 0xa4
  , BranchIfReferencesAreEqual = 0xa5
  , BranchIfReferencesAreNotEqual = 0xa6
  , Goto = 0xa7
  , JumpSubRoutine = 0xa8
  , ReturnFromSubRoutine = 0xa9
  , TableSwitch = 0xaa
  , LookupSwitch = 0xab
  , ReturnInteger = 0xac
  , ReturnLong = 0xad
  , ReturnFloat = 0xae
  , ReturnDouble = 0xaf
  , ReturnReference = 0xb0
  , ReturnVoid = 0xb1
  , GetStatic = 0xb2
  , PutStatic = 0xb3
  , GetField = 0xb4
  , PutField = 0xb5
  , InvokeVirtual = 0xb6
  , InvokeSpecial = 0xb7
  , InvokeStatic = 0xb8
  , InvokeInterface = 0xb9
  , New = 0xbb
  , NewArray = 0xbc
  , NewArrayOfReference = 0xbd
  , ArrayLength = 0xbe
  , ThrowReference = 0xbf
  , CheckCast = 0xc0
  , IsInstanceOf = 0xc1
  , MonitorEnter = 0xc2
  , MonitorExit = 0xc3
  , NewMultiDimentionalArray = 0xc5
  , BranchIfNull = 0xc6
  , BranchIfNotNull = 0xc7
};

#endif // _JAVAOPCODES__H_
