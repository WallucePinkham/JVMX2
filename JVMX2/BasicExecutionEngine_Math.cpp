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
  boost::intrusive_ptr<IJavaVariableType> pOperand2 = pVirtualMachineState->PopOperand();
  boost::intrusive_ptr<IJavaVariableType> pOperand1 = pVirtualMachineState->PopOperand();

  if (e_JavaVariableTypes::Float != pOperand2->GetVariableType())
  {
    throw InvalidStateException(__FUNCTION__ " - Expected a float on the operand stack.");
  }

  if (e_JavaVariableTypes::Float != pOperand1->GetVariableType())
  {
    throw InvalidStateException(__FUNCTION__ " - Expected a float on the operand stack.");
  }

  float value2 = boost::dynamic_pointer_cast<JavaFloat>(pOperand2)->ToHostFloat();
  float value1 = boost::dynamic_pointer_cast<JavaFloat>(pOperand1)->ToHostFloat();

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
  float float2 = boost::dynamic_pointer_cast<JavaFloat>(pVirtualMachineState->PopOperand())->ToHostFloat();
  float float1 = boost::dynamic_pointer_cast<JavaFloat>(pVirtualMachineState->PopOperand())->ToHostFloat();

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

  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(integer1 % integer2)));
}

void BasicExecutionEngine::ExecuteOpCodeLongRemainder(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  if (pVirtualMachineState->GetOperandStackSize() < 2)
  {
    throw InvalidStateException(__FUNCTION__ " - Not enough operands on the stack for long remainder operation.");
  }

  if (pVirtualMachineState->PeekOperand()->GetVariableType() != e_JavaVariableTypes::Long)
  {
    throw InvalidStateException(__FUNCTION__ " - Expected long on the operand stack.");
  }

  auto pLong2 = boost::dynamic_pointer_cast<JavaLong>(pVirtualMachineState->PopOperand());

  if (pVirtualMachineState->PeekOperand()->GetVariableType() != e_JavaVariableTypes::Long)
  {
    throw InvalidStateException(__FUNCTION__ " - Expected long on the operand stack.");
  }

  auto pLong1 = boost::dynamic_pointer_cast<JavaLong>(pVirtualMachineState->PopOperand());

  int64_t long2 = pLong2->ToHostInt64();
  int64_t long1 = pLong1->ToHostInt64();

  if (long2 == 0)
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaArithmeticException);
    return;
  }

  int64_t result = long1 % long2;

  pVirtualMachineState->PushOperand(new JavaLong(JavaLong::FromHostInt64(result)));
}

void BasicExecutionEngine::ExecuteOpCodeFloatRemainder(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  if (pVirtualMachineState->GetOperandStackSize() < 2)
  {
    throw InvalidStateException(__FUNCTION__ " - Not enough operands on the stack for float remainder operation.");
  }

  if (pVirtualMachineState->PeekOperand()->GetVariableType() != e_JavaVariableTypes::Float)
  {
    throw InvalidStateException(__FUNCTION__ " - Expected float on the operand stack.");
  }

  auto pFloat2 = boost::dynamic_pointer_cast<JavaFloat>(pVirtualMachineState->PopOperand());

  if (pVirtualMachineState->PeekOperand()->GetVariableType() != e_JavaVariableTypes::Float)
  {
    throw InvalidStateException(__FUNCTION__ " - Expected float on the operand stack.");
  }

  auto pFloat1 = boost::dynamic_pointer_cast<JavaFloat>(pVirtualMachineState->PopOperand());

  float float2 = pFloat2->ToHostFloat();
  float float1 = pFloat1->ToHostFloat();

  float result = 0.0f;

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

  if (isnan(float1) || isnan(float2))
  {
    result = NAN;
  }
  else if (isinf(float1) || isinf(float2) || float2 == 0.0f)
  {
    result = NAN;
  }
  else if (isinf(float1))
  {
    result = float1;
  }
  else if (isinf(float2))
  {
    result = float1;
  }
  else if (float1 == 0.0f)
  {
    result = float1;
  }
  else
  {
    // Calculate the floating point remainder
    int q = static_cast<int>(float1 / float2);
    result = float1 - (float2 * q);
  }

  pVirtualMachineState->PushOperand(new JavaFloat(JavaFloat::FromHostFloat(result)));
}

void BasicExecutionEngine::ExecuteOpCodeDoubleRemainder(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  if (pVirtualMachineState->GetOperandStackSize() < 2)
  {
    throw InvalidStateException(__FUNCTION__ " - Not enough operands on the stack for double remainder operation.");
  }

  if (pVirtualMachineState->PeekOperand()->GetVariableType() != e_JavaVariableTypes::Double)
  {
    throw InvalidStateException(__FUNCTION__ " - Expected double on the operand stack.");
  }

  auto pDouble2 = boost::dynamic_pointer_cast<JavaDouble>(pVirtualMachineState->PopOperand());

  if (pVirtualMachineState->PeekOperand()->GetVariableType() != e_JavaVariableTypes::Double)
  {
    throw InvalidStateException(__FUNCTION__ " - Expected double on the operand stack.");
  }

  auto pDouble1 = boost::dynamic_pointer_cast<JavaDouble>(pVirtualMachineState->PopOperand());

  double double2 = pDouble2->ToHostDouble();
  double double1 = pDouble1->ToHostDouble();

  double result = 0.0;

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

  if (isnan(double1) || isnan(double2))
  {
    result = NAN;
  }
  else if (isinf(double1) || isinf(double2) || double2 == 0.0)
  {
    result = NAN;
  }
  else if (isinf(double1))
  {
    result = double1;
  }
  else if (isinf(double2))
  {
    result = double1;
  }
  else if (double1 == 0.0)
  {
    result = double1;
  }
  else
  {
    // Calculate the floating point remainder
    int q = static_cast<int>(double1 / double2);
    result = double1 - (double2 * q);
  }

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
  boost::intrusive_ptr<JavaFloat> pFloat2 = boost::dynamic_pointer_cast<JavaFloat>(pVirtualMachineState->PopOperand());
  if (nullptr == pFloat2)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Expected first operand to be float type.");
  }

  boost::intrusive_ptr<JavaFloat> pFloat1 = boost::dynamic_pointer_cast<JavaFloat>(pVirtualMachineState->PopOperand());
  if (nullptr == pFloat1)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Expected second operand to be float type.");
  }

  int32_t result = 0;
  if (_isnan(pFloat1->ToHostFloat()) || _isnan(pFloat2->ToHostFloat()))
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
  boost::intrusive_ptr<JavaDouble> pDouble2 = boost::dynamic_pointer_cast<JavaDouble>(pVirtualMachineState->PopOperand());
  if (nullptr == pDouble2)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Expected first operand to be double type.");
  }

  boost::intrusive_ptr<JavaDouble> pDouble1 = boost::dynamic_pointer_cast<JavaDouble>(pVirtualMachineState->PopOperand());
  if (nullptr == pDouble1)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Expected second operand to be double type.");
  }

  int32_t result = 0;
  if (_isnan(pDouble1->ToHostDouble()) || _isnan(pDouble2->ToHostDouble()))
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

  int integer2 = GetIntegerFromOperandStack(pVirtualMachineState);
  int integer1 = GetIntegerFromOperandStack(pVirtualMachineState);

  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(integer1 + integer2)));

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

  int integer2 = GetIntegerFromOperandStack(pVirtualMachineState);
  int integer1 = GetIntegerFromOperandStack(pVirtualMachineState);

  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(integer1 * integer2)));
}

void BasicExecutionEngine::ExecuteOpCodeIntegerSubtract(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int integer2 = GetIntegerFromOperandStack(pVirtualMachineState);
  int integer1 = GetIntegerFromOperandStack(pVirtualMachineState);

  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(integer1 - integer2)));
}

void BasicExecutionEngine::ExecuteOpCodeFloatMultiply(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  boost::intrusive_ptr<JavaFloat> pFloat2 = boost::dynamic_pointer_cast<JavaFloat>(pVirtualMachineState->PopOperand());
  if (nullptr == pFloat2)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Expected first operand to be float type.");
  }

  boost::intrusive_ptr<JavaFloat> pFloat1 = boost::dynamic_pointer_cast<JavaFloat>(pVirtualMachineState->PopOperand());
  if (nullptr == pFloat1)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Expected second operand to be float type.");
  }

  pVirtualMachineState->PushOperand(new JavaFloat(JavaFloat::FromHostFloat(pFloat1->ToHostFloat() * pFloat2->ToHostFloat())));
}

void BasicExecutionEngine::ExecuteOpCodeNegateInteger(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int32_t value = GetIntegerFromOperandStack(pVirtualMachineState);
  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(value * -1)));
}

void BasicExecutionEngine::ExecuteOpCodeShiftIntegerLeft(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int32_t value2 = GetIntegerFromOperandStack(pVirtualMachineState);
  int32_t value1 = GetIntegerFromOperandStack(pVirtualMachineState);

  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(value1 << (value2 & 0x1F))));
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

  value2 = (value2 & 0x1F); // Get bottom 5 bits.

  // see this: http://stackoverflow.com/questions/5253194/implementing-logical-right-shift-in-c

  int size = sizeof(int32_t) * 8;
  int32_t result = (value1 >> value2) & ~(((0x1 << (size - 1)) >> value2) << 1);

  JVMX_ASSERT(result == ((uint32_t)value1) >> value2);

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

  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(integer1 / integer2)));
}

void BasicExecutionEngine::ExecuteOpCodeLongDivide(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  if (pVirtualMachineState->GetOperandStackSize() < 2)
  {
    throw InvalidStateException(__FUNCTION__ " - Not enough operands on the stack for long remainder operation.");
  }

  if (pVirtualMachineState->PeekOperand()->GetVariableType() != e_JavaVariableTypes::Long)
  {
    throw InvalidStateException(__FUNCTION__ " - Expected long on the operand stack.");
  }

  auto pLong2 = boost::dynamic_pointer_cast<JavaLong>(pVirtualMachineState->PopOperand());

  if (pVirtualMachineState->PeekOperand()->GetVariableType() != e_JavaVariableTypes::Long)
  {
    throw InvalidStateException(__FUNCTION__ " - Expected long on the operand stack.");
  }

  auto pLong1 = boost::dynamic_pointer_cast<JavaLong>(pVirtualMachineState->PopOperand());

  int64_t long2 = pLong2->ToHostInt64();
  int64_t long1 = pLong1->ToHostInt64();

  if (long2 == 0)
  {
    HelperClasses::ThrowJavaException(pVirtualMachineState, c_JavaArithmeticException);
    return;
  }

  int64_t result = long1 / long2;

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

  pVirtualMachineState->PushOperand(new JavaLong(JavaLong::FromHostInt64(long1 * long2)));

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

  pVirtualMachineState->PushOperand(new JavaLong(JavaLong::FromHostInt64(long1 + long2)));

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

  pVirtualMachineState->PushOperand(new JavaLong(JavaLong::FromHostInt64(value1 >> (value2 & 0x1F))));
}

void BasicExecutionEngine::ExecuteOpCodeShiftLongLeft(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int32_t value2 = GetIntegerFromOperandStack(pVirtualMachineState);
  int64_t value1 = GetLongFromOperandStack(pVirtualMachineState);

  pVirtualMachineState->PushOperand(new JavaLong(JavaLong::FromHostInt64(value1 << (value2 & 0x1F))));
}

void BasicExecutionEngine::ExecuteOpCodeDoubleDivide(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  boost::intrusive_ptr<IJavaVariableType> pOperand2 = pVirtualMachineState->PopOperand();
  boost::intrusive_ptr<IJavaVariableType> pOperand1 = pVirtualMachineState->PopOperand();

  if (e_JavaVariableTypes::Double != pOperand2->GetVariableType())
  {
    throw InvalidStateException(__FUNCTION__ " - Expected double as second parameter.");
  }

  if (e_JavaVariableTypes::Double != pOperand1->GetVariableType())
  {
    throw InvalidStateException(__FUNCTION__ " - Expected double as first parameter.");
  }

  double value1 = boost::dynamic_pointer_cast<JavaDouble>(pOperand1)->ToHostDouble();
  double value2 = boost::dynamic_pointer_cast<JavaDouble>(pOperand2)->ToHostDouble();

  double result = value1 / value2;

  pVirtualMachineState->PushOperand(new JavaDouble(JavaDouble::FromHostDouble(result)));
}

void BasicExecutionEngine::ExecuteOpCodeNegateDouble(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  if (e_JavaVariableTypes::Double != pVirtualMachineState->PeekOperand()->GetVariableType())
  {
    throw InvalidStateException(__FUNCTION__ " - Expected double on the operand stack.");
  }

  boost::intrusive_ptr<JavaDouble> pOperand = boost::dynamic_pointer_cast<JavaDouble>(pVirtualMachineState->PopOperand());

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

  pVirtualMachineState->PushOperand(new JavaLong(JavaLong::FromHostInt64(integer1 - integer2)));
}

void BasicExecutionEngine::ExecuteOpCodeORInteger(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int32_t integer2 = GetIntegerFromOperandStack(pVirtualMachineState);
  int32_t integer1 = GetIntegerFromOperandStack(pVirtualMachineState);

  pVirtualMachineState->PushOperand(new JavaInteger(JavaInteger::FromHostInt32(integer1 | integer2)));
}

void BasicExecutionEngine::ExecuteOpCodeFloatDivide(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  boost::intrusive_ptr<IJavaVariableType> pOperand2 = pVirtualMachineState->PopOperand();
  boost::intrusive_ptr<IJavaVariableType> pOperand1 = pVirtualMachineState->PopOperand();

  if (e_JavaVariableTypes::Float != pOperand2->GetVariableType())
  {
    throw InvalidStateException(__FUNCTION__ " - Expected float as second parameter.");
  }

  if (e_JavaVariableTypes::Float != pOperand1->GetVariableType())
  {
    throw InvalidStateException(__FUNCTION__ " - Expected float as first parameter.");
  }

  float value1 = boost::dynamic_pointer_cast<JavaFloat>(pOperand1)->ToHostFloat();
  float value2 = boost::dynamic_pointer_cast<JavaFloat>(pOperand2)->ToHostFloat();

  float result = value1 / value2;

  pVirtualMachineState->PushOperand(new JavaFloat(JavaFloat::FromHostFloat(result)));
}

void BasicExecutionEngine::ExecuteOpCodeDoubleMultiply(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  boost::intrusive_ptr<JavaDouble> pDouble2 = boost::dynamic_pointer_cast<JavaDouble>(pVirtualMachineState->PopOperand());
  if (nullptr == pDouble2)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Expected first operand to be float type.");
  }

  boost::intrusive_ptr<JavaDouble> pDouble1 = boost::dynamic_pointer_cast<JavaDouble>(pVirtualMachineState->PopOperand());
  if (nullptr == pDouble1)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Expected second operand to be float type.");
  }

  pVirtualMachineState->PushOperand(new JavaDouble(JavaDouble::FromHostDouble(pDouble1->ToHostDouble() * pDouble2->ToHostDouble())));
}

void BasicExecutionEngine::ExecuteOpCodeDoubleAdd(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (pVirtualMachineState->HasUserCodeStarted())
  {
    pVirtualMachineState->LogOperandStack();
  }
#endif // _DEBUG
  boost::intrusive_ptr<IJavaVariableType> pOperand2 = pVirtualMachineState->PopOperand();
  boost::intrusive_ptr<IJavaVariableType> pOperand1 = pVirtualMachineState->PopOperand();

  if (e_JavaVariableTypes::Double != pOperand2->GetVariableType())
  {
    throw InvalidStateException(__FUNCTION__ " - Expected a float on the operand stack.");
  }

  if (e_JavaVariableTypes::Double != pOperand1->GetVariableType())
  {
    throw InvalidStateException(__FUNCTION__ " - Expected a float on the operand stack.");
  }

  double value2 = boost::dynamic_pointer_cast<JavaDouble>(pOperand2)->ToHostDouble();
  double value1 = boost::dynamic_pointer_cast<JavaDouble>(pOperand1)->ToHostDouble();

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
  double double2 = boost::dynamic_pointer_cast<JavaDouble>(pVirtualMachineState->PopOperand())->ToHostDouble();
  double double1 = boost::dynamic_pointer_cast<JavaDouble>(pVirtualMachineState->PopOperand())->ToHostDouble();

  pVirtualMachineState->PushOperand(new JavaDouble(JavaDouble::FromHostDouble(double1 - double2)));
}

void BasicExecutionEngine::ExecuteOpCodeNegateLong(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  if (e_JavaVariableTypes::Long != pVirtualMachineState->PeekOperand()->GetVariableType())
  {
    throw InvalidStateException(__FUNCTION__ " - Expected long on the operand stack.");
  }

  boost::intrusive_ptr<JavaLong> pOperand = boost::dynamic_pointer_cast<JavaLong>(pVirtualMachineState->PopOperand());
  int64_t negatedValue = -pOperand->ToHostInt64();
  pVirtualMachineState->PushOperand(new JavaLong(JavaLong::FromHostInt64(negatedValue)));
}

void BasicExecutionEngine::ExecuteOpCodeNegateFloat(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  if (e_JavaVariableTypes::Float != pVirtualMachineState->PeekOperand()->GetVariableType())
  {
    throw InvalidStateException(__FUNCTION__ " - Expected float on the operand stack.");
  }

  boost::intrusive_ptr<JavaFloat> pOperand = boost::dynamic_pointer_cast<JavaFloat>(pVirtualMachineState->PopOperand());
  float negatedValue = -pOperand->ToHostFloat();
  pVirtualMachineState->PushOperand(new JavaFloat(JavaFloat::FromHostFloat(negatedValue)));
}

void BasicExecutionEngine::ExecuteOpCodeShiftLongRightArithmetic(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState)
{
  int32_t value2 = GetIntegerFromOperandStack(pVirtualMachineState);
  int64_t value1 = GetLongFromOperandStack(pVirtualMachineState);

  uint64_t uvalue1 = static_cast<uint64_t>(value1);
  int64_t result = static_cast<int64_t>(uvalue1 >> (value2 & 0x3F));

  pVirtualMachineState->PushOperand(new JavaLong(JavaLong::FromHostInt64(result)));
}
