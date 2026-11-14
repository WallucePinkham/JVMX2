#include <cstdlib>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <atomic>
#include <limits>
#include <memory>
#include <vector>
#include <map>
#include <thread>
#include <utility>

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

namespace
{
  void ValidateMathOperands(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, e_JavaVariableTypes expectedType, uint8_t operandCount)
  {
    if (pVirtualMachineState->GetOperandStackSize() < operandCount)
    {
      throw InvalidStateException("Not enough operands on the stack for math operation.");
    }

    for (uint8_t operandIndex = 1; operandIndex <= operandCount; ++operandIndex)
    {
      boost::intrusive_ptr<IJavaVariableType> pOperand = pVirtualMachineState->PeekOperandFromBack(operandIndex);
      if (nullptr == pOperand || expectedType != pOperand->GetVariableType())
      {
        throw InvalidStateException("Unexpected operand type on the stack for math operation.");
      }
    }
  }

  template <typename T>
  boost::intrusive_ptr<T> PopValidatedMathOperand(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, e_JavaVariableTypes expectedType)
  {
    ValidateMathOperands(pVirtualMachineState, expectedType, 1);
    boost::intrusive_ptr<T> pOperand = boost::dynamic_pointer_cast<T>(pVirtualMachineState->PopOperand());
    if (nullptr == pOperand)
    {
      throw InvalidStateException("Unexpected operand type on the stack for math operation.");
    }

    return pOperand;
  }

  template <typename T>
  std::pair<boost::intrusive_ptr<T>, boost::intrusive_ptr<T> > PopValidatedMathOperands(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, e_JavaVariableTypes expectedType)
  {
    ValidateMathOperands(pVirtualMachineState, expectedType, 2);
    boost::intrusive_ptr<T> pOperand2 = boost::dynamic_pointer_cast<T>(pVirtualMachineState->PopOperand());
    boost::intrusive_ptr<T> pOperand1 = boost::dynamic_pointer_cast<T>(pVirtualMachineState->PopOperand());
    if (nullptr == pOperand1 || nullptr == pOperand2)
    {
      throw InvalidStateException("Unexpected operand type on the stack for math operation.");
    }

    return std::make_pair(pOperand1, pOperand2);
  }

  int32_t WrapToInt32(uint32_t bits)
  {
    int32_t result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
  }

  int64_t WrapToInt64(uint64_t bits)
  {
    int64_t result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
  }
}


void BasicExecutionEngine::ExecuteOpCodeORLong(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  int64_t long2 = GetLongFromOperandStack(pVirtualMachineState);
  int64_t long1 = GetLongFromOperandStack(pVirtualMachineState);

  pVirtualMachineState->PushOperand(new JavaLong(JavaLong::FromHostInt64(long1 | long2)));

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeFloatAdd(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
  std::pair<boost::intrusive_ptr<JavaFloat>, boost::intrusive_ptr<JavaFloat> > operands =
    PopValidatedMathOperands<JavaFloat>(pVirtualMachineState, e_JavaVariableTypes::Float);
  float value2 = operands.second->ToHostFloat();
  float value1 = operands.first->ToHostFloat();

  pVirtualMachineState->PushOperand(new JavaFloat(JavaFloat::FromHostFloat(value1 + value2)));

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeFloatSubtract(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  std::pair<boost::intrusive_ptr<JavaFloat>, boost::intrusive_ptr<JavaFloat> > operands =
    PopValidatedMathOperands<JavaFloat>(pVirtualMachineState, e_JavaVariableTypes::Float);
  float float2 = operands.second->ToHostFloat();
  float float1 = operands.first->ToHostFloat();

  pVirtualMachineState->PushOperand(new JavaFloat(JavaFloat::FromHostFloat(float1 - float2)));
}

void BasicExecutionEngine::ExecuteOpCodeIntegerRemainder(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int integer2 = GetIntegerFromOperandStack(pVirtualMachineState);
  int integer1 = GetIntegerFromOperandStack(pVirtualMachineState);

  if (integer2 == 0)
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaArithmeticException);
    return;
  }

  if (integer1 == (std::numeric_limits<int32_t>::min)() && integer2 == -1)
  {
    pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(0)));
    return;
  }

  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(integer1 % integer2)));
}

void BasicExecutionEngine::ExecuteOpCodeLongRemainder(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  std::pair<boost::intrusive_ptr<JavaLong>, boost::intrusive_ptr<JavaLong> > operands =
    PopValidatedMathOperands<JavaLong>(pVirtualMachineState, e_JavaVariableTypes::Long);
  int64_t long2 = operands.second->ToHostInt64();
  int64_t long1 = operands.first->ToHostInt64();

  if (long2 == 0)
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaArithmeticException);
    return;
  }

  int64_t result = (long1 == (std::numeric_limits<int64_t>::min)() && long2 == -1)
    ? 0
    : long1 % long2;

  pVirtualMachineState->PushOperand(new JavaLong(JavaLong::FromHostInt64(result)));
}

void BasicExecutionEngine::ExecuteOpCodeFloatRemainder(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  std::pair<boost::intrusive_ptr<JavaFloat>, boost::intrusive_ptr<JavaFloat> > operands =
    PopValidatedMathOperands<JavaFloat>(pVirtualMachineState, e_JavaVariableTypes::Float);
  float result = std::fmod(operands.first->ToHostFloat(), operands.second->ToHostFloat());

  /*
  The result of an frem instruction is governed by these rules :
  * If either value1' or value2' is NaN, the result is NaN.
    * If neither value1' nor value2' is NaN, the sign of the result equals
    the sign of the dividend.
    * If the dividend is an infinity or the divisor is a zero or both, the
    result is NaN.
    * If the dividend is finite and the divisor is an infinity, the result
    equals the dividend.
    * If the dividend is a zero and the divisor is finite, the result equals
    the dividend.
    
    * In the remaining cases, where neither operand is an infinity,
    a zero, or NaN, the floating - point remainder result from
    a dividend value1' and a divisor value2' is defined by the
    mathematical relation result = value1' - (value2' * q), where
    q is an integer that is negative only if value1' / value2' is
    negative and positive only if value1' / value2' is positive, and
    whose magnitude is as large as possible without exceeding the
    magnitude of the true mathematical quotient of value1' and
    value2'.
    */

  pVirtualMachineState->PushOperand(new JavaFloat(JavaFloat::FromHostFloat(result)));
}

void BasicExecutionEngine::ExecuteOpCodeDoubleRemainder(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  std::pair<boost::intrusive_ptr<JavaDouble>, boost::intrusive_ptr<JavaDouble> > operands =
    PopValidatedMathOperands<JavaDouble>(pVirtualMachineState, e_JavaVariableTypes::Double);
  double result = std::fmod(operands.first->ToHostDouble(), operands.second->ToHostDouble());

  /*
  The result of an frem instruction is governed by these rules :
  * If either value1' or value2' is NaN, the result is NaN.
    * If neither value1' nor value2' is NaN, the sign of the result equals
    the sign of the dividend.
    * If the dividend is an infinity or the divisor is a zero or both, the
    result is NaN.
    * If the dividend is finite and the divisor is an infinity, the result
    equals the dividend.
    * If the dividend is a zero and the divisor is finite, the result equals
    the dividend.

    * In the remaining cases, where neither operand is an infinity,
    a zero, or NaN, the floating - point remainder result from
    a dividend value1' and a divisor value2' is defined by the
    mathematical relation result = value1' - (value2' * q), where
    q is an integer that is negative only if value1' / value2' is
    negative and positive only if value1' / value2' is positive, and
    whose magnitude is as large as possible without exceeding the
    magnitude of the true mathematical quotient of value1' and
    value2'.
    */

  pVirtualMachineState->PushOperand(new JavaDouble(JavaDouble::FromHostDouble(result)));
}

void BasicExecutionEngine::ExecuteOpCodeFloatComparisonL(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(CompareFloatValues(pVirtualMachineState, -1))));
}

void BasicExecutionEngine::ExecuteOpCodeFloatComparisonG(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(CompareFloatValues(pVirtualMachineState, 1))));
}

int BasicExecutionEngine::CompareFloatValues(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, int nanResult)
{
  std::pair<boost::intrusive_ptr<JavaFloat>, boost::intrusive_ptr<JavaFloat> > operands =
    PopValidatedMathOperands<JavaFloat>(pVirtualMachineState, e_JavaVariableTypes::Float);
  boost::intrusive_ptr<JavaFloat> pFloat1 = operands.first;
  boost::intrusive_ptr<JavaFloat> pFloat2 = operands.second;

  int32_t result = 0;
  if (std::isnan(pFloat1->ToHostFloat()) || std::isnan(pFloat2->ToHostFloat()))
  {
    result = nanResult;
  }
  else if (*pFloat1 > *pFloat2)
  {
    result = 1;
  }
  else if (*pFloat1 < *pFloat2)
  {
    result = -1;
  }
  else
  {
    result = 0;
  }

  return result;
}

int BasicExecutionEngine::CompareDoubleValues(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, int nanResult)
{
  std::pair<boost::intrusive_ptr<JavaDouble>, boost::intrusive_ptr<JavaDouble> > operands =
    PopValidatedMathOperands<JavaDouble>(pVirtualMachineState, e_JavaVariableTypes::Double);
  boost::intrusive_ptr<JavaDouble> pDouble1 = operands.first;
  boost::intrusive_ptr<JavaDouble> pDouble2 = operands.second;

  int32_t result = 0;
  if (std::isnan(pDouble1->ToHostDouble()) || std::isnan(pDouble2->ToHostDouble()))
  {
    result = nanResult;
  }
  else if (*pDouble1 > *pDouble2)
  {
    result = 1;
  }
  else if (*pDouble1 < *pDouble2)
  {
    result = -1;
  }
  else
  {
    result = 0;
  }

  return result;
}

void BasicExecutionEngine::ExecuteOpCodeIntegerAdd(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  int32_t integer2 = GetIntegerFromOperandStack(pVirtualMachineState);
  int32_t integer1 = GetIntegerFromOperandStack(pVirtualMachineState);

  uint32_t resultBits = static_cast<uint32_t>(integer1) + static_cast<uint32_t>(integer2);
  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(WrapToInt32(resultBits))));

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeIntegerMultiply(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  int32_t integer2 = GetIntegerFromOperandStack(pVirtualMachineState);
  int32_t integer1 = GetIntegerFromOperandStack(pVirtualMachineState);

  uint32_t resultBits = static_cast<uint32_t>(integer1) * static_cast<uint32_t>(integer2);
  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(WrapToInt32(resultBits))));
}

void BasicExecutionEngine::ExecuteOpCodeIntegerSubtract(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int32_t integer2 = GetIntegerFromOperandStack(pVirtualMachineState);
  int32_t integer1 = GetIntegerFromOperandStack(pVirtualMachineState);

  uint32_t resultBits = static_cast<uint32_t>(integer1) - static_cast<uint32_t>(integer2);
  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(WrapToInt32(resultBits))));
}

void BasicExecutionEngine::ExecuteOpCodeFloatMultiply(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  std::pair<boost::intrusive_ptr<JavaFloat>, boost::intrusive_ptr<JavaFloat> > operands =
    PopValidatedMathOperands<JavaFloat>(pVirtualMachineState, e_JavaVariableTypes::Float);

  pVirtualMachineState->PushOperand(new JavaFloat(JavaFloat::FromHostFloat(operands.first->ToHostFloat() * operands.second->ToHostFloat())));
}

void BasicExecutionEngine::ExecuteOpCodeNegateInteger(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int32_t value = GetIntegerFromOperandStack(pVirtualMachineState);
  uint32_t resultBits = 0u - static_cast<uint32_t>(value);
  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(WrapToInt32(resultBits))));
}

void BasicExecutionEngine::ExecuteOpCodeShiftIntegerLeft(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int32_t value2 = GetIntegerFromOperandStack(pVirtualMachineState);
  int32_t value1 = GetIntegerFromOperandStack(pVirtualMachineState);

  uint32_t resultBits = static_cast<uint32_t>(value1) << (value2 & 0x1F);
  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(WrapToInt32(resultBits))));
}

void BasicExecutionEngine::ExecuteOpCodeShiftIntegerRightArithmetic(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int32_t value2 = GetIntegerFromOperandStack(pVirtualMachineState);
  int32_t value1 = GetIntegerFromOperandStack(pVirtualMachineState);

  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(value1 >> (value2 & 0x1F))));
}

void BasicExecutionEngine::ExecuteOpCodeShiftIntegerRightLogical(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  int32_t value2 = GetIntegerFromOperandStack(pVirtualMachineState);
  int32_t value1 = GetIntegerFromOperandStack(pVirtualMachineState);

  uint32_t resultBits = static_cast<uint32_t>(value1) >> (value2 & 0x1F);
  int32_t result = WrapToInt32(resultBits);

  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(result)));

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeIntegerDivide(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int integer2 = GetIntegerFromOperandStack(pVirtualMachineState);
  int integer1 = GetIntegerFromOperandStack(pVirtualMachineState);

  if (0 == integer2)
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaArithmeticException);
    return;
  }

  if (integer1 == (std::numeric_limits<int32_t>::min)() && integer2 == -1)
  {
    pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(integer1)));
    return;
  }

  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(integer1 / integer2)));
}

void BasicExecutionEngine::ExecuteOpCodeLongDivide(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  std::pair<boost::intrusive_ptr<JavaLong>, boost::intrusive_ptr<JavaLong> > operands =
    PopValidatedMathOperands<JavaLong>(pVirtualMachineState, e_JavaVariableTypes::Long);
  int64_t long2 = operands.second->ToHostInt64();
  int64_t long1 = operands.first->ToHostInt64();

  if (long2 == 0)
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaArithmeticException);
    return;
  }

  int64_t result = (long1 == (std::numeric_limits<int64_t>::min)() && long2 == -1)
    ? long1
    : long1 / long2;

  pVirtualMachineState->PushOperand(new JavaLong(JavaLong::FromHostInt64(result)));
}

void BasicExecutionEngine::ExecuteOpCodeXORInteger(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  int integer2 = GetIntegerFromOperandStack(pVirtualMachineState);
  int integer1 = GetIntegerFromOperandStack(pVirtualMachineState);

  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(integer1 ^ integer2)));

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeLongComparison(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int64_t long2 = static_cast<int64_t>(GetLongFromOperandStack(pVirtualMachineState));
  int64_t long1 = static_cast<int64_t>(GetLongFromOperandStack(pVirtualMachineState));

  int32_t result = 0;
  if (long1 > long2)
  {
    result = 1;
  }
  else if (long2 > long1)
  {
    result = -1;
  }
  else
  {
    result = 0;
  }

  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(result)));
}

void BasicExecutionEngine::ExecuteOpCodeXORLong(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  int64_t long2 = GetLongFromOperandStack(pVirtualMachineState);
  int64_t long1 = GetLongFromOperandStack(pVirtualMachineState);

  pVirtualMachineState->PushOperand(new JavaLong(JavaLong::FromHostInt64(long1 ^ long2)));

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeANDLong(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  int64_t long2 = GetLongFromOperandStack(pVirtualMachineState);
  int64_t long1 = GetLongFromOperandStack(pVirtualMachineState);

  pVirtualMachineState->PushOperand(new JavaLong(JavaLong::FromHostInt64(long1 & long2)));

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeLongMultiply(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  int64_t long2 = GetLongFromOperandStack(pVirtualMachineState);
  int64_t long1 = GetLongFromOperandStack(pVirtualMachineState);

  uint64_t resultBits = static_cast<uint64_t>(long1) * static_cast<uint64_t>(long2);
  pVirtualMachineState->PushOperand(new JavaLong(JavaLong::FromHostInt64(WrapToInt64(resultBits))));

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeLongAdd(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  int64_t long2 = GetLongFromOperandStack(pVirtualMachineState);
  int64_t long1 = GetLongFromOperandStack(pVirtualMachineState);

  uint64_t resultBits = static_cast<uint64_t>(long1) + static_cast<uint64_t>(long2);
  pVirtualMachineState->PushOperand(new JavaLong(JavaLong::FromHostInt64(WrapToInt64(resultBits))));

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeShiftLongRight(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int32_t value2 = GetIntegerFromOperandStack(pVirtualMachineState);
  int64_t value1 = GetLongFromOperandStack(pVirtualMachineState);

  uint64_t resultBits = static_cast<uint64_t>(value1) >> (value2 & 0x3F);
  pVirtualMachineState->PushOperand(new JavaLong(JavaLong::FromHostInt64(WrapToInt64(resultBits))));
}

void BasicExecutionEngine::ExecuteOpCodeShiftLongLeft(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int32_t value2 = GetIntegerFromOperandStack(pVirtualMachineState);
  int64_t value1 = GetLongFromOperandStack(pVirtualMachineState);

  uint64_t resultBits = static_cast<uint64_t>(value1) << (value2 & 0x3F);
  pVirtualMachineState->PushOperand(new JavaLong(JavaLong::FromHostInt64(WrapToInt64(resultBits))));
}

void BasicExecutionEngine::ExecuteOpCodeDoubleDivide(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  std::pair<boost::intrusive_ptr<JavaDouble>, boost::intrusive_ptr<JavaDouble> > operands =
    PopValidatedMathOperands<JavaDouble>(pVirtualMachineState, e_JavaVariableTypes::Double);
  double value1 = operands.first->ToHostDouble();
  double value2 = operands.second->ToHostDouble();

  double result = value1 / value2;

  pVirtualMachineState->PushOperand(new JavaDouble(JavaDouble::FromHostDouble(result)));
}

void BasicExecutionEngine::ExecuteOpCodeNegateDouble(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  boost::intrusive_ptr<JavaDouble> pOperand = PopValidatedMathOperand<JavaDouble>(pVirtualMachineState, e_JavaVariableTypes::Double);

  double negatedValue = (-pOperand->ToHostDouble()); // Apply operator -()

  pVirtualMachineState->PushOperand(new JavaDouble(JavaDouble::FromHostDouble(negatedValue)));
}

void BasicExecutionEngine::ExecuteOpCodeANDInteger(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG

  int32_t int2 = GetIntegerFromOperandStack(pVirtualMachineState);
  int32_t int1 = GetIntegerFromOperandStack(pVirtualMachineState);

  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(int1 & int2)));

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeLongSubtract(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int64_t integer2 = GetLongFromOperandStack(pVirtualMachineState);
  int64_t integer1 = GetLongFromOperandStack(pVirtualMachineState);

  uint64_t resultBits = static_cast<uint64_t>(integer1) - static_cast<uint64_t>(integer2);
  pVirtualMachineState->PushOperand(new JavaLong(JavaLong::FromHostInt64(WrapToInt64(resultBits))));
}

void BasicExecutionEngine::ExecuteOpCodeORInteger(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int32_t integer2 = GetIntegerFromOperandStack(pVirtualMachineState);
  int32_t integer1 = GetIntegerFromOperandStack(pVirtualMachineState);

  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(integer1 | integer2)));
}

void BasicExecutionEngine::ExecuteOpCodeFloatDivide(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  std::pair<boost::intrusive_ptr<JavaFloat>, boost::intrusive_ptr<JavaFloat> > operands =
    PopValidatedMathOperands<JavaFloat>(pVirtualMachineState, e_JavaVariableTypes::Float);
  float value1 = operands.first->ToHostFloat();
  float value2 = operands.second->ToHostFloat();

  float result = value1 / value2;

  pVirtualMachineState->PushOperand(new JavaFloat(JavaFloat::FromHostFloat(result)));
}

void BasicExecutionEngine::ExecuteOpCodeDoubleMultiply(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  std::pair<boost::intrusive_ptr<JavaDouble>, boost::intrusive_ptr<JavaDouble> > operands =
    PopValidatedMathOperands<JavaDouble>(pVirtualMachineState, e_JavaVariableTypes::Double);

  pVirtualMachineState->PushOperand(new JavaDouble(JavaDouble::FromHostDouble(operands.first->ToHostDouble() * operands.second->ToHostDouble())));
}

void BasicExecutionEngine::ExecuteOpCodeDoubleAdd(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
  std::pair<boost::intrusive_ptr<JavaDouble>, boost::intrusive_ptr<JavaDouble> > operands =
    PopValidatedMathOperands<JavaDouble>(pVirtualMachineState, e_JavaVariableTypes::Double);
  double value2 = operands.second->ToHostDouble();
  double value1 = operands.first->ToHostDouble();

  pVirtualMachineState->PushOperand(new JavaDouble(JavaDouble::FromHostDouble(value1 + value2)));

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
}

void BasicExecutionEngine::ExecuteOpCodeDoubleComparisonL(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(CompareDoubleValues(pVirtualMachineState, -1))));
}

void BasicExecutionEngine::ExecuteOpCodeDoubleComparisonG(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(CompareDoubleValues(pVirtualMachineState, 1))));
}

void BasicExecutionEngine::ExecuteOpCodeDoubleSubtract(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  std::pair<boost::intrusive_ptr<JavaDouble>, boost::intrusive_ptr<JavaDouble> > operands =
    PopValidatedMathOperands<JavaDouble>(pVirtualMachineState, e_JavaVariableTypes::Double);
  double double2 = operands.second->ToHostDouble();
  double double1 = operands.first->ToHostDouble();

  pVirtualMachineState->PushOperand(new JavaDouble(JavaDouble::FromHostDouble(double1 - double2)));
}

void BasicExecutionEngine::ExecuteOpCodeNegateLong(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  boost::intrusive_ptr<JavaLong> pOperand = PopValidatedMathOperand<JavaLong>(pVirtualMachineState, e_JavaVariableTypes::Long);
  uint64_t resultBits = 0u - static_cast<uint64_t>(pOperand->ToHostInt64());
  int64_t negatedValue = WrapToInt64(resultBits);
  pVirtualMachineState->PushOperand(new JavaLong(JavaLong::FromHostInt64(negatedValue)));
}

void BasicExecutionEngine::ExecuteOpCodeNegateFloat(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  boost::intrusive_ptr<JavaFloat> pOperand = PopValidatedMathOperand<JavaFloat>(pVirtualMachineState, e_JavaVariableTypes::Float);
  float negatedValue = -pOperand->ToHostFloat();
  pVirtualMachineState->PushOperand(new JavaFloat(JavaFloat::FromHostFloat(negatedValue)));
}

void BasicExecutionEngine::ExecuteOpCodeShiftLongRightArithmetic(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int32_t value2 = GetIntegerFromOperandStack(pVirtualMachineState);
  int64_t value1 = GetLongFromOperandStack(pVirtualMachineState);

  uint32_t shiftCount = static_cast<uint32_t>(value2) & 0x3F;
  uint64_t resultBits = static_cast<uint64_t>(value1) >> shiftCount;
  if (value1 < 0 && shiftCount != 0)
  {
    resultBits |= (~static_cast<uint64_t>(0)) << (64 - shiftCount);
  }
  int64_t result = WrapToInt64(resultBits);

  pVirtualMachineState->PushOperand(new JavaLong(JavaLong::FromHostInt64(result)));
}
