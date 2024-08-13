
#include "GlobalConstants.h"

#include "Stream.h"
#include "JavaTypes.h"
#include "TypeParser.h"

#include "IClassLibrary.h"
#include "ILogger.h"
#include "IExecutionEngine.h"

#include "NullPointerException.h"
#include "InvalidStateException.h"
#include "InvalidArgumentException.h"
#include "UnsupportedTypeException.h"
#include "NotImplementedException.h"

#include "JavaExceptionConstants.h"

#include "JavaCodeAttribute.h"
#include "ClassAttributeCode.h"
#include "CodeSegmentDataBuffer.h"
#include "ThreadManager.h"
#include "VirtualMachine.h"

#include "DefaultClassLoader.h"
#include "JavaNativeInterface.h"

#include "HelperTypes.h"
#include "HelperConversion.h"

#include "IMemoryManager.h"
#include "GlobalCatalog.h"
#include "ObjectReference.h"
#include "ObjectRegistryLocalMachine.h"

#include "BasicVirtualMachineState.h"
#include "ClassAttributeSourceFile.h"
#include "CodeAttributeLineNumberTable.h"


extern const JavaString c_ClassInitialisationMethodName = JavaString::FromCString(JVMX_T("<clinit>"));
extern const JavaString c_ClassInitialisationMethodType = JavaString::FromCString(JVMX_T("()V"));
extern const JavaString c_InstanceInitialisationMethodName = JavaString::FromCString(JVMX_T("<init>"));
extern const JavaString c_InstanceInitialisationMethodType = JavaString::FromCString(JVMX_T("()V"));

extern const JavaString c_StringInitialisationMethodTypeWithArrayIntIntBool = JavaString::FromCString(JVMX_T("([CIIZ)V"));
extern const JavaString c_StringInitialisationMethodTypeWithArrayOnly = JavaString::FromCString(JVMX_T("([C)V"));


extern const JavaString c_JavaLangClassInitialisationMethodType = JavaString::FromCString(JVMX_T("(Ljava/lang/Object;)V"));

extern const JavaString c_JavaLangClassName = JavaString::FromCString(JVMX_T("java/lang/Class"));
extern const JavaString c_SyntheticField_ClassName = JavaString::FromCString(JVMX_T("__class"));

const uint32_t c_NullReferenceValue = UINT32_MAX;

BasicVirtualMachineState::BasicVirtualMachineState(std::shared_ptr<VirtualMachine> pVM, bool hasUserCodeStarted)
  : m_pVM(pVM)
  , m_LocalVariableStackFramePointer(0)
  , m_isShuttingDown(false)
  , m_CurrentClassAndMethodName(JavaString::EmptyString())
  , m_ExitCode(0)
  , m_isPaused(false)
  , m_isPausing(false)
  , m_ExceptionOccurred(false)
  , m_pException(nullptr)
  , m_CallStackDepth(0)
  , m_StackLevel(0)
  , m_isInterrupted(false)
  , m_NativeExecutionCount(0)
  , m_hasUserCodeStarted(hasUserCodeStarted)
{
  m_CurrentRegisters.m_ProgramCounter = 0;
  m_CurrentRegisters.m_pCodeSegmentStart = nullptr;
  m_CurrentRegisters.m_CodeSegmentLength = 0;
}

BasicVirtualMachineState::~BasicVirtualMachineState() JVMX_NOEXCEPT
{
  ReleaseMemory();
}

void BasicVirtualMachineState::ReleaseMemory()
{
  m_OperandStack.clear();
  m_LocalVariableStack.clear();

  while (!m_RegisterStack.empty())
  {
    m_RegisterStack.pop_back();
  }

  m_CurrentDisplayCallStackEntry.m_ClassName = JavaString::EmptyString();
  m_CurrentDisplayCallStackEntry.m_MethodName = JavaString::EmptyString();
  m_CurrentDisplayCallStackEntry.m_MethodType = JavaString::EmptyString();
  m_CurrentDisplayCallStackEntry.m_ProgramCounter = 0;

  m_DisplayCallStack.clear();
}

bool BasicVirtualMachineState::IsShuttingDown() const
{
  return m_isShuttingDown;
}

int BasicVirtualMachineState::GetExitCode() const
{
  return m_ExitCode;
}

void BasicVirtualMachineState::RunAllFinalizers()
{
  std::shared_ptr<IGarbageCollector> pGC = GlobalCatalog::GetInstance().Get("GarbageCollector");
  pGC->RunAllFinalizers(shared_from_this());
}

void BasicVirtualMachineState::SetExceptionThrown(boost::intrusive_ptr<ObjectReference> pException)
{
  m_ExceptionOccurred = true;
  m_pException = pException;
}

bool BasicVirtualMachineState::HasExceptionOccurred() const
{
  return m_ExceptionOccurred;
}

boost::intrusive_ptr<ObjectReference> BasicVirtualMachineState::GetException()
{
  return m_pException;
}

void BasicVirtualMachineState::ResetException()
{
  m_ExceptionOccurred = false;
  m_pException = nullptr;
}

void BasicVirtualMachineState::PushMonitor(std::shared_ptr<Lockable> pMutex)
{
  m_MutexStack.push(pMutex);
}

std::shared_ptr<Lockable> BasicVirtualMachineState::PopMonitor()
{
  std::shared_ptr<Lockable> pMutex = m_MutexStack.top();
  m_MutexStack.pop();
  return pMutex;
}

void BasicVirtualMachineState::PushAndZeroCallStackDepth()
{
  m_CallStackDepthStack.push(m_CallStackDepth);
  m_CallStackDepth = 0;
}

uint16_t BasicVirtualMachineState::PopCallStackDepth()
{
  m_CallStackDepth = m_CallStackDepthStack.top();
  m_CallStackDepthStack.pop();
  return m_CallStackDepth;
}

uint16_t BasicVirtualMachineState::GetCallStackDepth()
{
  return m_CallStackDepth;
}

uint16_t BasicVirtualMachineState::IncrementCallStackDepth()
{
  return ++m_CallStackDepth;
}

uint16_t BasicVirtualMachineState::DecrementCallStackDepth()
{
  return --m_CallStackDepth;
}

void BasicVirtualMachineState::SetJavaNativeInterface(std::weak_ptr<JavaNativeInterface> pJNI)
{
  m_pJNI = pJNI;
#if _DEBUG // Do this here, because this method is called in the new thread
  m_ThreadId = std::hash<std::thread::id>{} (std::this_thread::get_id());
#endif
}

ThreadInfo BasicVirtualMachineState::ReturnCurrentThreadInfo()
{
  return m_pVM->ReturnCurrentThreadObject();
}

void BasicVirtualMachineState::RegisterNativeMethods(std::shared_ptr<JavaNativeInterface> pJNI)
{
  return m_pVM->RegisterNativeMethods(pJNI);
}

void BasicVirtualMachineState::AddThread(std::shared_ptr<boost::thread> pNewThread, boost::intrusive_ptr<ObjectReference> pObject, const std::shared_ptr<IVirtualMachineState>& pNewState)
{
  return m_pVM->AddThread(pNewThread, pObject, pNewState);
}

boost::intrusive_ptr<ObjectReference> BasicVirtualMachineState::GetCallStackOfClassObjects()
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (HasUserCodeStarted())
  {
    LogCallStack();
  }
#endif

  size_t arraySize = m_DisplayCallStack.size() - 1;
  boost::intrusive_ptr<ObjectReference> pResult = CreateArray(e_JavaArrayTypes::Reference, arraySize);

  size_t arrayIndex = 0;

  for (auto it = m_DisplayCallStack.crbegin(); it != m_DisplayCallStack.crend(); ++it)
  {
    boost::intrusive_ptr<JavaString> pName = new JavaString(it->m_ClassName);

    if (!pName->IsEmpty())
    {
      boost::intrusive_ptr<ObjectReference> pClassObject = CreateJavaLangClassFromClassName(pName);
      pResult->GetContainedArray()->SetAt(arrayIndex, pClassObject.get());
      ++arrayIndex;
    }

    JVMX_ASSERT(arrayIndex <= arraySize);
  }

  return pResult;
}

const boost::intrusive_ptr<JavaString> BasicVirtualMachineState::GetSoureFileName(const std::shared_ptr<MethodInfo> pMethodInfo) const
{
  boost::intrusive_ptr<JavaString> pFileName = nullptr;

  int classAttributeCount = pMethodInfo->GetClass()->GetAttributeCount();
  for (int i = 0; i < classAttributeCount; ++i)
  {
    auto classAttribute = pMethodInfo->GetClass()->GetAttribute(i);
    if (e_JavaAttributeTypeSourceFile == classAttribute->JavaCodeAttribute::GetType())
    {
      auto pSourceFileAttribute = std::dynamic_pointer_cast<ClassAttributeSourceFile>(classAttribute);
      std::shared_ptr<ConstantPool> pConstantPool = pMethodInfo->GetClass()->GetConstantPool();
      std::shared_ptr<ConstantPoolEntry> pConstant = pConstantPool->GetConstant(pSourceFileAttribute->ClassAttributeSourceFile::GetIndex());

      pFileName = pConstant->AsString();
      JVMX_ASSERT(nullptr != pFileName);
    }
  }

  return pFileName;
}

uint16_t BasicVirtualMachineState::GetLineNumber(int stackPos)
{
  uint16_t lineNumber = 0;

  for (auto attribute : m_MethodInfoStack[stackPos]->GetAttributes())
  {
    if (e_JavaAttributeTypeCode != attribute->GetType())
    {
      continue;
    }

    std::shared_ptr<ClassAttributeCode> pAttributeCode = std::dynamic_pointer_cast<ClassAttributeCode>(attribute);
    for (auto codeAttribute : pAttributeCode->GetAttributeList())
    {
      if (e_JavaAttributeTypeLineNumberTable != codeAttribute->GetType())
      {
        continue;
      }

      std::shared_ptr<CodeAttributeLineNumberTable> pLineNumberTable = std::dynamic_pointer_cast<CodeAttributeLineNumberTable>(codeAttribute);

      for (uint16_t i = 0; i < pLineNumberTable->GetNumberOfLineNumbers(); ++i)
      {
        if (m_RegisterStack[stackPos].m_ProgramCounter >= pLineNumberTable->GetStartPositionAt(i))
        {
          if (i >= pLineNumberTable->GetNumberOfLineNumbers() - 1)
          {
            lineNumber = pLineNumberTable->GetLineNumberAt(i);
            break;
          }

          if (m_RegisterStack[stackPos].m_ProgramCounter < pLineNumberTable->GetStartPositionAt(i + 1))
          {
            lineNumber = pLineNumberTable->GetLineNumberAt(i);
            break;
          }
        }
      }

      if (lineNumber > 0)
      {
        break;
      }
    }

    if (lineNumber > 0)
    {
      break;
    }
  }

  return lineNumber;
}

boost::intrusive_ptr<ObjectReference> BasicVirtualMachineState::GetCallStackOfStackTraceElements()
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (HasUserCodeStarted())
  {
    LogCallStack();
  }
#endif

  // Preparation in terms of StackTraceElement.
  std::shared_ptr<JavaClass> pClassOfStackTraceElement = LoadClass(JavaString::FromCString("java/lang/StackTraceElement"));
  if (nullptr == pClassOfStackTraceElement)
  {
    throw InvalidStateException(__FUNCTION__ " - Could not load class StackTraceElement.");
  }

  if (!pClassOfStackTraceElement->IsInitialsed())
  {
    InitialiseClass(*pClassOfStackTraceElement->GetName());
  }

  static const JavaString c_MethodType = JavaString::FromCString("(Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;Z)V");
  std::shared_ptr<MethodInfo> pConstructorMethodInfo = ResolveMethod(pClassOfStackTraceElement.get(), c_InstanceInitialisationMethodName, c_MethodType);
  // Done with prep work.

  size_t arraySize = m_MethodInfoStack.size();// - 1;
  boost::intrusive_ptr<ObjectReference> pResult = CreateArray(e_JavaArrayTypes::Reference, arraySize);

  size_t arrayIndex = 0;

  for (int stackPos = m_MethodInfoStack.size() - 1; stackPos >= 0; --stackPos)
  {

    if (!m_MethodInfoStack[stackPos]->GetFullName().IsEmpty())
    {
      boost::intrusive_ptr<ObjectReference> pStackTraceElement = CreateObject(pClassOfStackTraceElement);

      // Call Constructor
      //////////////////////////////////////////////////////////////////////////
      if (pConstructorMethodInfo->IsSynchronised())
      {
        PushMonitor(pStackTraceElement->GetContainedObject()->MonitorEnter(GetCurrentClassAndMethodName().ToUtf8String().c_str()));
      }

      boost::intrusive_ptr<JavaString> pFileName = GetSoureFileName(m_MethodInfoStack[stackPos]);
      uint16_t lineNumber = GetLineNumber(stackPos);

      PushOperand(pStackTraceElement);
      if (nullptr == pFileName)
      {
        PushOperand(boost::intrusive_ptr<ObjectReference>(new ObjectReference(nullptr))); // FileName (treat this as unknown for now).
      }
      else
      {
        PushOperand(boost::intrusive_ptr<ObjectReference>(CreateStringObject(*pFileName)));
      }

      if (0 == lineNumber)
      {
        PushOperand(boost::intrusive_ptr<JavaInteger>(new JavaInteger(JavaInteger::FromHostInt32(-1)))); // Line Number (treat this as unknown for now).
      }
      else
      {
        PushOperand(boost::intrusive_ptr<JavaInteger>(new JavaInteger(JavaInteger::FromHostInt32(static_cast<int32_t>(lineNumber)))));
      }

      PushOperand(CreateStringObject(*m_MethodInfoStack[stackPos]->GetClass()->GetName()));
      PushOperand(CreateStringObject(*m_MethodInfoStack[stackPos]->GetName()));
      PushOperand(boost::intrusive_ptr<JavaBool>(new JavaBool(JavaBool::FromBool(m_MethodInfoStack[stackPos]->IsNative()))));

      ExecuteMethod(*pClassOfStackTraceElement->GetName(), c_InstanceInitialisationMethodName, c_MethodType, pConstructorMethodInfo);
      //////////////////////////////////////////////////////////////////////////

      pResult->GetContainedArray()->SetAt(arrayIndex, pStackTraceElement.get());
      ++arrayIndex;
    }

    JVMX_ASSERT(arrayIndex <= arraySize);
  }

  return pResult;
}

std::shared_ptr<ILogger> BasicVirtualMachineState::GetLogger()
{
  if (nullptr == m_pLogger)
  {
    m_pLogger = GlobalCatalog::GetInstance().Get("Logger");
  }

  return m_pLogger;
}

std::shared_ptr<IClassLibrary> BasicVirtualMachineState::GetClassLibrary()
{
  if (nullptr == m_pClassLibrary)
  {
    m_pClassLibrary = GlobalCatalog::GetInstance().Get("ClassLibrary");
  }

  return m_pClassLibrary;
}

void BasicVirtualMachineState::Execute(const JavaString& startingClassName, const JavaString& methodName, const JavaString& methodType)
{
  std::shared_ptr<MethodInfo> pInitialMethod = GetMethodByNameAndType(startingClassName, methodName, methodType, GetClassLibrary());

  if (nullptr == pInitialMethod)
  {
    if (methodName != c_ClassInitialisationMethodName && methodName != c_InstanceInitialisationMethodName)
    {
      throw InvalidStateException(__FUNCTION__ " - Initial Method was not found");
    }
    else
    {
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
      if (HasUserCodeStarted())
      {
        GetLogger()->LogDebug("Class %s does not have a method called %s", startingClassName.ToUtf8String().c_str(), methodName.ToUtf8String().c_str());
      }
#endif // (_DEBUG) && defined(JVMX_LOG_VERBOSE)

      auto pClass = LoadClass(startingClassName);
      pClass->SetInitialised();

      return;
    }
  }

  //   if ( !pInitialMethod->IsStatic( ) )
  //   {
  //     throw InvalidStateException( __FUNCTION__ " - Initial Method was not static" );
  //   }

  if (!pInitialMethod->GetClass()->IsInitialsed() && methodName == c_ClassInitialisationMethodName)
  {
    pInitialMethod->GetClass()->SetInitialising();
  }

  ExecuteMethod(startingClassName, methodName, *(pInitialMethod->GetType()), pInitialMethod);

  if (pInitialMethod->GetClass()->IsInitialsing() && methodName == c_ClassInitialisationMethodName)
  {
    pInitialMethod->GetClass()->SetInitialised();
  }

  //PopState();
}

void BasicVirtualMachineState::Execute(const MethodInfo& method)
{
  ClassAttributeCode code = FindCodeAttribute(method);

  CodeSegmentDataBuffer codeSegment(code.GetCode());
  m_CurrentRegisters.m_pCodeSegmentStart = codeSegment.GetRawDataPointer();
  m_CurrentRegisters.m_CodeSegmentLength = codeSegment.GetByteLength();
  m_CurrentRegisters.m_ProgramCounter = 0;

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (HasUserCodeStarted())
  {
    //GetLogger()->LogDebug( "Executing Method: %s::%s", m_CurrentDisplayCallStackEntry.m_ClassName.ToUtf8String().c_str(), method.GetName()->ToUtf8String().c_str() );
    GetLogger()->LogDebug("Executing Method: %s::%s", method.GetClass()->GetName()->ToUtf8String().c_str(), method.GetName()->ToUtf8String().c_str());
    AssertValid();
  }
#endif // _DEBUG

  std::shared_ptr<IExecutionEngine> pEngine = GlobalCatalog::GetInstance().Get("ExecutionEngine");
  pEngine->Run(shared_from_this());

#ifdef _DEBUG
  AssertValid();
#endif // _DEBUG
}

std::shared_ptr<MethodInfo> BasicVirtualMachineState::GetMethodByNameAndType(const JavaString& className, const JavaString& methodName, const JavaString& methodType, std::shared_ptr<IClassLibrary> pConstantPool)
{
  std::shared_ptr<JavaClass> pClass = GetClassByName(pConstantPool, className);

  std::shared_ptr<MethodInfo> pMethod = pClass->GetMethodByNameAndType(methodName, methodType);
  if (nullptr != pMethod)
  {
    return pMethod;
  }

  return nullptr;
}

std::shared_ptr<JavaClass> BasicVirtualMachineState::GetClassByName(std::shared_ptr<IClassLibrary> pConstantPool, const JavaString& className)
{
  std::shared_ptr<JavaClass> pClass = pConstantPool->FindClass(className);
  if (nullptr == pClass)
  {
    pClass = LoadClass(className);
    if (nullptr == pClass)
    {
      throw InvalidArgumentException(__FUNCTION__ " - Could not load class.");
    }
  }

  return pClass;
}

ClassAttributeCode BasicVirtualMachineState::FindCodeAttribute(const MethodInfo& method)
{
  const CodeAttributeList& attributes = method.GetAttributes();
  for (auto attrib : attributes)
  {
    if (c_AttributeNameCode == attrib->GetName())
    {
      return attrib->ToCode();
    }
  }

  throw InvalidStateException(__FUNCTION__ " - Code Attribute not found.");
}

void BasicVirtualMachineState::PushMethodStack(const JavaString& newClassName, const JavaString& newMethodName, const JavaString& newMethodType)
{
  m_DisplayCallStack.push_back(m_CurrentDisplayCallStackEntry);

  UpdateCurrentClassName(newClassName);
  m_CurrentDisplayCallStackEntry.m_MethodName = newMethodName;
  m_CurrentDisplayCallStackEntry.m_MethodType = newMethodType;
  m_CurrentDisplayCallStackEntry.m_ProgramCounter = m_CurrentRegisters.m_ProgramCounter;

  m_CurrentClassAndMethodName = BuildCurrentClassAndMethodName();
}

bool BasicVirtualMachineState::IsInitialRun() const JVMX_NOEXCEPT
{
  return m_CurrentDisplayCallStackEntry.m_ClassName.IsEmpty() && m_CurrentDisplayCallStackEntry.m_MethodName.IsEmpty();
}

void BasicVirtualMachineState::PopMethodStack()
{
  m_CurrentDisplayCallStackEntry = m_DisplayCallStack.back();
  m_DisplayCallStack.pop_back();

  m_CurrentClassAndMethodName = BuildCurrentClassAndMethodName();
}

uintptr_t BasicVirtualMachineState::GetProgramCounter() const
{
  return m_CurrentRegisters.m_ProgramCounter;
}

void BasicVirtualMachineState::AdvanceProgramCounter(int byteCount)
{
  if (m_CurrentRegisters.m_ProgramCounter + byteCount < 0 || m_CurrentRegisters.m_ProgramCounter + byteCount > m_CurrentRegisters.m_CodeSegmentLength)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Trying to jump to an address outside of the current code block.");
  }

  m_CurrentRegisters.m_ProgramCounter += byteCount;
}

size_t BasicVirtualMachineState::GetCodeSegmentLength() const
{
  return m_CurrentRegisters.m_CodeSegmentLength;
}

const uint8_t* BasicVirtualMachineState::GetCodeSegmentStart() const
{
  return m_CurrentRegisters.m_pCodeSegmentStart;
}

bool BasicVirtualMachineState::CanReadBytes(int byteCount) const
{
  return m_CurrentRegisters.m_ProgramCounter + byteCount <= m_CurrentRegisters.m_CodeSegmentLength;
}

std::shared_ptr<ConstantPoolEntry> BasicVirtualMachineState::GetConstantFromCurrentClass(ConstantPoolIndex index)
{
  return GetClassLibrary()->GetConstant(m_CurrentDisplayCallStackEntry.m_ClassName, index);
}

std::shared_ptr<MethodInfo> BasicVirtualMachineState::GetMethod(size_t index)
{
  return GetClassLibrary()->GetMethod(m_CurrentDisplayCallStackEntry.m_ClassName, index);
}

bool BasicVirtualMachineState::IsClassInitialised(const JavaString& className)
{
  return GetClassLibrary()->IsClassInitalised(className);
}

std::shared_ptr<JavaClass> BasicVirtualMachineState::GetCurrentClass()
{
  return GetClassLibrary()->FindClass(m_CurrentDisplayCallStackEntry.m_ClassName);
}

void BasicVirtualMachineState::PushState(const JavaString& newClassName, const JavaString& newMethodName, const JavaString& newMethodType, std::shared_ptr<MethodInfo> pNewMethodInfo)
{
  PushMethodStack(newClassName, newMethodName, newMethodType);
  m_MethodInfoStack.push_back(pNewMethodInfo);
  m_RegisterStack.push_back(m_CurrentRegisters);
  m_LocalVariableStackFrameStack.push(m_LocalVariableStackFramePointer);
}

void BasicVirtualMachineState::PopState()
{
  m_LocalVariableStack.resize(m_LocalVariableStackFramePointer);
  m_LocalVariableStackFramePointer = m_LocalVariableStackFrameStack.top();
  m_LocalVariableStackFrameStack.pop();

  m_MethodInfoStack.pop_back();
  m_CurrentRegisters = m_RegisterStack.back();
  m_RegisterStack.pop_back();

  PopMethodStack();
}

const CodeAttributeStackMapTable* BasicVirtualMachineState::GetCurrentStackMap()
{
  return GetMethodByNameAndType(m_CurrentDisplayCallStackEntry.m_ClassName, m_CurrentDisplayCallStackEntry.m_MethodName, m_CurrentDisplayCallStackEntry.m_MethodType, GetClassLibrary())->GetFrame();
}

const ClassAttributeCode* BasicVirtualMachineState::GetCurrentCodeInfo()
{
  return GetMethodByNameAndType(m_CurrentDisplayCallStackEntry.m_ClassName, m_CurrentDisplayCallStackEntry.m_MethodName, m_CurrentDisplayCallStackEntry.m_MethodType, GetClassLibrary())->GetCodeInfo();
}

const JavaString& BasicVirtualMachineState::GetCurrentClassAndMethodName() const
{
  return m_CurrentClassAndMethodName;
}

JavaString BasicVirtualMachineState::BuildCurrentClassAndMethodName() const
{
  size_t length = m_CurrentDisplayCallStackEntry.m_ClassName.GetLengthInCodePoints() + 2 + m_CurrentDisplayCallStackEntry.m_MethodName.GetLengthInCodePoints() + sizeof(uint16_t);
  char* pBuffer = new char[length * sizeof(char16_t)];
  char* pPos = pBuffer;

  try
  {
    memcpy(pPos, m_CurrentDisplayCallStackEntry.m_ClassName.ToCharacterArray(), m_CurrentDisplayCallStackEntry.m_ClassName.GetLengthInBytes());
    pPos += m_CurrentDisplayCallStackEntry.m_ClassName.GetLengthInBytes();

    memcpy(pPos, u"::", 2 * sizeof(char16_t));
    pPos += 2 * sizeof(char16_t);

    memcpy(pBuffer + m_CurrentDisplayCallStackEntry.m_ClassName.GetLengthInBytes() + 2 * sizeof(char16_t), m_CurrentDisplayCallStackEntry.m_MethodName.ToCharacterArray(), m_CurrentDisplayCallStackEntry.m_MethodName.GetLengthInBytes());
    pPos += m_CurrentDisplayCallStackEntry.m_MethodName.GetLengthInBytes();

    pPos[0] = ('\0');
    pPos[1] = ('\0');

    JavaString result = JavaString::FromCString(reinterpret_cast<const char16_t*>(pBuffer));

    delete[] pBuffer;
    pBuffer = nullptr;

    return result;
  }
  catch (...)
  {
    delete[] pBuffer;
    throw;
  }
}

void BasicVirtualMachineState::SetCodeSegment(const ClassAttributeCode* pCodeInfo)
{
  m_CurrentRegisters.m_pCodeSegmentStart = pCodeInfo->GetCode().ToByteArray();
  m_CurrentRegisters.m_CodeSegmentLength = pCodeInfo->GetCode().GetByteLength();
  m_CurrentRegisters.m_ProgramCounter = 0;
}

void BasicVirtualMachineState::PushOperand(const boost::intrusive_ptr<IJavaVariableType>& pOperand)
{
  if (nullptr == pOperand)
  {
    throw InvalidArgumentException(__FUNCTION__ " - Invalid argument passed. Operand was NULL");
  }

  // TODO: There should be a way to avoid the string copies here. This is too expensive and kills performance.

  m_OperandStack.push_back({ pOperand, GetProgramCounter(), m_DisplayCallStack.size(), GetCurrentClassName(), GetCurrentMethodName(), GetCurrentMethodType() });
}

boost::intrusive_ptr<IJavaVariableType> BasicVirtualMachineState::PopOperand()
{
  if (m_OperandStack.size() < 1)
  {
    throw InvalidStateException(__FUNCTION__ " - PopOperand called on empty operand stack.");
  }

  boost::intrusive_ptr<IJavaVariableType> pOperand = PeekOperand();
  m_OperandStack.pop_back();

  return pOperand;
}

const JavaString& BasicVirtualMachineState::GetCurrentClassName() const
{
  return m_CurrentDisplayCallStackEntry.m_ClassName;
}

const JavaString& BasicVirtualMachineState::GetCurrentMethodName() const
{
  return m_CurrentDisplayCallStackEntry.m_MethodName;
}

const JavaString& BasicVirtualMachineState::GetCurrentMethodType() const
{
  return m_CurrentDisplayCallStackEntry.m_MethodType;
}

boost::intrusive_ptr<ObjectReference> BasicVirtualMachineState::SetupLocalVariables(std::shared_ptr<MethodInfo> pMethodInfo)
{
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Setting up %u local variables", pMethodInfo->GetCodeInfo()->GetLocalVariableArraySizeIncludingPassedParameters());
    LogOperandStack();
  }
#endif

  std::vector<boost::intrusive_ptr<IJavaVariableType> > paramArray = PopulateParameterArrayFromOperandStack(pMethodInfo);
  boost::intrusive_ptr<ObjectReference> pObject = nullptr;

  if (!pMethodInfo->IsStatic())
  {
    pObject = boost::dynamic_pointer_cast<ObjectReference>(paramArray[0]);

    if (nullptr == pObject)
    {
#if defined (_DEBUG)
      LogCallStack();
      LogOperandStack();
#endif

      throw InvalidStateException(__FUNCTION__ " - Expected object pointer to be valid.");
    }
  }

  SetupLocalVariables(pMethodInfo, pObject, paramArray);

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (HasUserCodeStarted())
  {
    LogLocalVariables();
  }
#endif // _DEBUG

  return pObject;
}

// void BasicVirtualMachineState::SetupLocalVariables( std::shared_ptr<MethodInfo> pMethodInfo, boost::intrusive_ptr<ObjectReference> pObject, const std::vector<boost::intrusive_ptr<IJavaVariableType> > &paramArray )
// {
//   InitialiseLocalVariables( pMethodInfo->GetCodeInfo()->GetLocalVariableArraySizeIncludingPassedParameters(), GetLocalVariableTable( pMethodInfo ), pMethodInfo->GetClass()->GetConstantPool() );
//   TypeParser::ParsedMethodType parsedType = TypeParser::ParseMethodType( *pMethodInfo->GetType() );
//
//   size_t paramIndex = 1;
//   uint16_t localVariableIndex = 1;
//   uint16_t nonStaticMethodModifier = 1;
//
//   if ( pMethodInfo->IsStatic() )
//   {
//     localVariableIndex = 0;
//     paramIndex = 0;
//     nonStaticMethodModifier = 0;
//   }
//
// #ifdef _DEBUG
//   GetLogger()->LogDebug( "Method %s descriptor is: %s", pMethodInfo->GetName()->ToByteArray(), pMethodInfo->GetType()->ToByteArray() );
// #endif // _DEBUG
//
//   while ( paramIndex < paramArray.size() )
//   {
//     // Now Assign the Value
//     switch ( parsedType.parameters[ paramIndex - nonStaticMethodModifier ]->At( 0 ) )
//     {
//       case c_JavaTypeSpecifierByte:
//         SetLocalVariable( localVariableIndex, boost::dynamic_pointer_cast<JavaByte>(paramArray[ paramIndex ]) );
//         break;
//
//       case c_JavaTypeSpecifierChar:
//       {
//         if ( paramArray[ paramIndex ]->GetVariableType() == e_JavaVariableTypes::Integer )
//         {
//           SetLocalVariable( localVariableIndex, TypeParser::DownCastFromInteger( boost::dynamic_pointer_cast<JavaInteger>(paramArray[ paramIndex ]), e_JavaVariableTypes::Char ) );
//         }
//         else
//         {
//           SetLocalVariable( localVariableIndex, boost::dynamic_pointer_cast<JavaChar>(paramArray[ paramIndex ]) );
//         }
//       }
//       break;
//
//       case c_JavaTypeSpecifierInteger:
//         if ( paramArray[ paramIndex ]->GetVariableType() != e_JavaVariableTypes::Integer )
//         {
//           throw InvalidStateException( __FUNCTION__ " - Expected integer on the operand stack." );
//         }
//         SetLocalVariable( localVariableIndex, boost::dynamic_pointer_cast<JavaInteger>(paramArray[ paramIndex ]) );
//         break;
//
//       case c_JavaTypeSpecifierShort:
//         SetLocalVariable( localVariableIndex, boost::dynamic_pointer_cast<JavaShort>(paramArray[ paramIndex ]) );
//         break;
//
//       case c_JavaTypeSpecifierBool:
//         if ( paramArray[ paramIndex ]->GetVariableType() == e_JavaVariableTypes::Integer )
//         {
//           SetLocalVariable( localVariableIndex, TypeParser::DownCastFromInteger( boost::dynamic_pointer_cast<JavaInteger>(paramArray[ paramIndex ]), e_JavaVariableTypes::Bool ) );
//         }
//         else
//         {
//           SetLocalVariable( localVariableIndex, boost::dynamic_pointer_cast<JavaBool>(paramArray[ paramIndex ]) );
//         }
//         break;
//
//       case c_JavaTypeSpecifierFloat:
//         if ( paramArray[ paramIndex ]->GetVariableType() != e_JavaVariableTypes::Float )
//         {
//           throw InvalidStateException( __FUNCTION__ " - Expected float on the operand stack." );
//         }
//
//         SetLocalVariable( localVariableIndex, boost::dynamic_pointer_cast<JavaFloat>(paramArray[ paramIndex ]) );
//         break;
//
//       case c_JavaTypeSpecifierDouble:
//         SetLocalVariable( localVariableIndex, boost::dynamic_pointer_cast<JavaDouble>(paramArray[ paramIndex ]) );
//         --localVariableIndex;
//         break;
//
//       case c_JavaTypeSpecifierLong:
//         SetLocalVariable( localVariableIndex, boost::dynamic_pointer_cast<JavaLong>(paramArray[ paramIndex ]) );
//         --localVariableIndex;
//         break;
//
//       case c_JavaTypeSpecifierArray:
//         if ( paramArray[ paramIndex ]->IsNull() )
//         {
//           SetLocalVariable( localVariableIndex,new ObjectReference(nullptr) );
//         }
//         else
//         {
//           SetLocalVariable( localVariableIndex, boost::dynamic_pointer_cast<ObjectReference>(paramArray[ paramIndex ]) );
//         }
//         break;
//
//       case c_JavaTypeSpecifierReference:
//       {
//         SetupLocalVariableTypeReference( paramArray, paramIndex, localVariableIndex );
//       }
//       break;
//
//       default:
//         throw UnsupportedTypeException( __FUNCTION__ " - Unknown type found." );
//         break;
//     }
//
//     ++localVariableIndex;
//     ++paramIndex;
//   }
//
//   if ( !pMethodInfo->IsStatic() )
//   {
//     SetLocalVariable( 0, pObject );
//   }
//
// #if 0
//   LogLocalVariables();
// #endif // 0
//
//   return;
// }

void BasicVirtualMachineState::SetupLocalVariables(std::shared_ptr<MethodInfo> pMethodInfo, boost::intrusive_ptr<ObjectReference> pObject, const std::vector<boost::intrusive_ptr<IJavaVariableType> >& paramArray)
{
  //if ( !pMethodInfo->IsNative() )
  //{
  InitialiseLocalVariables(pMethodInfo->GetCodeInfo()->GetLocalVariableArraySizeIncludingPassedParameters(), GetLocalVariableTable(pMethodInfo), pMethodInfo->GetClass()->GetConstantPool());
  //}
  //else
  //{
  //  InitialiseLocalVariables( paramArray.size(), nullptr, pMethodInfo->GetClass()->GetConstantPool() );
  //}

  TypeParser::ParsedMethodType parsedType = TypeParser::ParseMethodType(*pMethodInfo->GetType());

  size_t paramIndex = 1;
  uint16_t localVariableIndex = 1;
  uint16_t nonStaticMethodModifier = 1;

  if (pMethodInfo->IsStatic())
  {
    localVariableIndex = 0;
    paramIndex = 0;
    nonStaticMethodModifier = 0;
  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Method %s descriptor is: %s", pMethodInfo->GetName()->ToUtf8String().c_str(), pMethodInfo->GetType()->ToUtf8String().c_str());
    GetLogger()->LogDebug("Param Array:");
    for (size_t i = 0; i < paramArray.size(); ++i)
    {
      GetLogger()->LogDebug("\tParameter %d is: %s", i, paramArray[i]->ToString().ToUtf8String().c_str());
    }
    GetLogger()->LogDebug("Parsed types:");
    for (size_t i = 0; i < parsedType.parameters.size(); ++i)
    {
      GetLogger()->LogDebug("\tParameter %d type: %s", i, parsedType.parameters[i]->ToString().ToUtf8String().c_str());
    }
  }
#endif // _DEBUG

  while (paramIndex < paramArray.size())
  {
    // Now Assign the Value
    switch (parsedType.parameters[paramIndex - nonStaticMethodModifier]->At(0))
    {
    case c_JavaTypeSpecifierByte:
      if (paramArray[paramIndex]->GetVariableType() == e_JavaVariableTypes::Integer)
      {
        SetLocalVariable(localVariableIndex, TypeParser::DownCastFromInteger(boost::dynamic_pointer_cast<JavaInteger>(paramArray[paramIndex]), e_JavaVariableTypes::Byte));
      }
      else
      {
        SetLocalVariable(localVariableIndex, boost::dynamic_pointer_cast<JavaByte>(paramArray[paramIndex]));
      }
      break;

    case c_JavaTypeSpecifierChar:
    {
      if (paramArray[paramIndex]->GetVariableType() == e_JavaVariableTypes::Integer)
      {
        SetLocalVariable(localVariableIndex, TypeParser::DownCastFromInteger(boost::dynamic_pointer_cast<JavaInteger>(paramArray[paramIndex]), e_JavaVariableTypes::Char));
      }
      else
      {
        SetLocalVariable(localVariableIndex, boost::dynamic_pointer_cast<JavaChar>(paramArray[paramIndex]));
      }
    }
    break;

    case c_JavaTypeSpecifierInteger:
      if (paramArray[paramIndex]->GetVariableType() != e_JavaVariableTypes::Integer)
      {
        throw InvalidStateException(__FUNCTION__ " - Expected integer on the operand stack.");
      }
      SetLocalVariable(localVariableIndex, boost::dynamic_pointer_cast<JavaInteger>(paramArray[paramIndex]));
      break;

    case c_JavaTypeSpecifierShort:
      if (paramArray[paramIndex]->GetVariableType() == e_JavaVariableTypes::Integer)
      {
        SetLocalVariable(localVariableIndex, TypeParser::DownCastFromInteger(boost::dynamic_pointer_cast<JavaInteger>(paramArray[paramIndex]), e_JavaVariableTypes::Short));
      }
      else
      {
        SetLocalVariable(localVariableIndex, boost::dynamic_pointer_cast<JavaShort>(paramArray[paramIndex]));
      }
      break;

    case c_JavaTypeSpecifierBool:
      if (paramArray[paramIndex]->GetVariableType() == e_JavaVariableTypes::Integer)
      {
        SetLocalVariable(localVariableIndex, TypeParser::DownCastFromInteger(boost::dynamic_pointer_cast<JavaInteger>(paramArray[paramIndex]), e_JavaVariableTypes::Bool));
      }
      else
      {
        SetLocalVariable(localVariableIndex, boost::dynamic_pointer_cast<JavaBool>(paramArray[paramIndex]));
      }
      break;

    case c_JavaTypeSpecifierFloat:
      if (paramArray[paramIndex]->GetVariableType() != e_JavaVariableTypes::Float)
      {
        throw InvalidStateException(__FUNCTION__ " - Expected float on the operand stack.");
      }

      SetLocalVariable(localVariableIndex, boost::dynamic_pointer_cast<JavaFloat>(paramArray[paramIndex]));
      break;

    case c_JavaTypeSpecifierDouble:
      SetLocalVariable(localVariableIndex, boost::dynamic_pointer_cast<JavaDouble>(paramArray[paramIndex]));
      //--localVariableIndex;
      ++localVariableIndex;
      break;

    case c_JavaTypeSpecifierLong:
      SetLocalVariable(localVariableIndex, boost::dynamic_pointer_cast<JavaLong>(paramArray[paramIndex]));
      //--localVariableIndex;
      ++localVariableIndex;
      break;

    case c_JavaTypeSpecifierArray:
      if (paramArray[paramIndex]->IsNull())
      {
        SetLocalVariable(localVariableIndex, boost::intrusive_ptr<ObjectReference>(new ObjectReference(nullptr)));
      }
      else
      {
        SetLocalVariable(localVariableIndex, boost::dynamic_pointer_cast<ObjectReference>(paramArray[paramIndex]));
      }
      break;

    case c_JavaTypeSpecifierReference:
    {
      SetupLocalVariableTypeReference(paramArray, paramIndex, localVariableIndex);
    }
    break;

    default:
      throw UnsupportedTypeException(__FUNCTION__ " - Unknown type found.");
      break;
    }

    ++localVariableIndex;
    ++paramIndex;
  }

  if (!pMethodInfo->IsStatic())
  {
    SetLocalVariable(0, pObject);
  }

#if 0
  LogLocalVariables();
#endif // 0

  return;
}

std::shared_ptr<CodeAttributeLocalVariableTable> BasicVirtualMachineState::GetLocalVariableTable(std::shared_ptr<MethodInfo> pMethodInfo)
{
  const CodeAttributeList& alist = pMethodInfo->GetCodeInfo()->GetAttributeList();
  for (auto it = alist.begin(); it != alist.end(); ++it)
  {
    if (e_JavaAttributeTypeLocalVariableTable == (*it)->GetType())
    {
      return std::dynamic_pointer_cast<CodeAttributeLocalVariableTable>(*it);
    }
  }

  return nullptr;
}

std::shared_ptr<IVirtualMachineState> BasicVirtualMachineState::CreateNewState()
{
  return std::make_shared<BasicVirtualMachineState>(m_pVM, m_hasUserCodeStarted);
}

void BasicVirtualMachineState::SetupLocalVariableTypeReference(const std::vector<boost::intrusive_ptr<IJavaVariableType> >& paramArray, size_t paramIndex, uint16_t localVariableIndex)
{
  auto pOperand = paramArray[paramIndex];
  if (pOperand->IsNull())
  {
    SetLocalVariable(localVariableIndex, boost::intrusive_ptr<ObjectReference>(new ObjectReference(nullptr)));
  }
  else
  {
    // TODO: More options here.

    if (e_JavaVariableTypes::Object == pOperand->GetVariableType())
    {
      auto pObjectParam = boost::dynamic_pointer_cast<ObjectReference>(pOperand);
      SetLocalVariable(localVariableIndex, pObjectParam);
    }
    else if (e_JavaVariableTypes::Array == pOperand->GetVariableType())
    {
      auto pArray = boost::dynamic_pointer_cast<ObjectReference>(pOperand);
      SetLocalVariable(localVariableIndex, pArray);
    }
    else if (e_JavaVariableTypes::ClassReference == pOperand->GetVariableType())
    {
      auto pClassRef = boost::dynamic_pointer_cast<JavaClassReference>(pOperand);
      auto pJavaLangClass = CreateJavaLangClassFromClassName(pClassRef->GetClassFile()->GetName());
      SetLocalVariable(localVariableIndex, pJavaLangClass);

      //SetLocalVariable( localVariableIndex, pClassRef );
    }
    else
    {
      throw NotImplementedException(__FUNCTION__ " - Not yet implemented");
    }
  }
}

void BasicVirtualMachineState::SetLocalVariable(uint16_t localVariableIndex, boost::intrusive_ptr<IJavaVariableType> pValue)
{
  if (nullptr == pValue)
  {
    throw NullPointerException(__FUNCTION__ " - Value argument was NULL.");
  }

  m_LocalVariableStack[m_LocalVariableStackFramePointer + localVariableIndex].m_pValue = pValue;
}

void BasicVirtualMachineState::NameLocalVariable(uint16_t localVariableIndex, boost::intrusive_ptr<JavaString> pName)
{
  if (nullptr == pName)
  {
    throw NullPointerException(__FUNCTION__ " - Value argument was NULL.");
  }

  m_LocalVariableStack[m_LocalVariableStackFramePointer + localVariableIndex].m_pName = pName;
}

boost::intrusive_ptr<IJavaVariableType> BasicVirtualMachineState::GetLocalVariable(uint16_t localVariableIndex)
{
  return m_LocalVariableStack.at(m_LocalVariableStackFramePointer + localVariableIndex).m_pValue;
}

const boost::intrusive_ptr<JavaString>& BasicVirtualMachineState::GetLocalVariableName(uint16_t localVariableIndex)
{
  return m_LocalVariableStack.at(m_LocalVariableStackFramePointer + localVariableIndex).m_pName;
}


std::shared_ptr<JavaClass> BasicVirtualMachineState::LoadClass(const DataBuffer& classData)
{
  DefaultClassLoader loader;
  auto pClass = loader.LoadClass(BigEndianStream::FromDataBuffer(classData));

  if (nullptr != pClass)
  {
    GetClassLibrary()->AddClass(pClass);
  }

  return pClass;
}

std::shared_ptr<JavaClass> BasicVirtualMachineState::LoadClass(const JavaString& className, const JavaString& path)
{
  DefaultClassLoader loader;

  std::shared_ptr<JavaClass> pClass = GetClassLibrary()->FindClass(className);
  if (nullptr != pClass)
  {
    return pClass;
  }

  try
  {

    //auto pEnv = m_pJNI.lock()->GetEnvironment();
    //pEnv->

    if (!path.IsEmpty())
    {
      loader.AddSearchPath(path.ToCharacterArray());
    }

    JavaString finalClassName = className.Append(JVMX_T(".class"));
    pClass = loader.LoadClass(finalClassName.ToUtf16String().c_str());
  }
  catch (FileInvalidException&)
  {
    // TODO: Fix this.
    pClass = nullptr;
    throw;
  }
  catch (FileDoesNotExistException&)
  {
    //ThrowJavaException( "ClassNotFoundException" );
    pClass = nullptr;
    throw;
  }

  if (nullptr != pClass)
  {
    GetClassLibrary()->AddClass(pClass);
  }

  return pClass;
}

boost::intrusive_ptr<IJavaVariableType> BasicVirtualMachineState::PeekOperand()
{
  if (0 == m_OperandStack.size())
  {
    return nullptr;
  }

  return m_OperandStack.back().pOperand;
}

void BasicVirtualMachineState::InitialiseLocalVariables(size_t numberofLocalVariables, std::shared_ptr<CodeAttributeLocalVariableTable> pLocalVariableTable, std::shared_ptr<ConstantPool> pConstantPool)
{
#ifdef _DEBUG
  const size_t debugNumberOfVariables = numberofLocalVariables;
  const size_t debugStackSize = m_LocalVariableStack.size();
#endif // _DEBUG

  m_LocalVariableStackFramePointer = m_LocalVariableStack.size();
  m_LocalVariableStack.resize(m_LocalVariableStack.size() + numberofLocalVariables);

#ifdef _DEBUG
  JVMX_ASSERT(m_LocalVariableStack.size() == debugStackSize + debugNumberOfVariables);
#endif // _DEBUG

  for (size_t index = 0; index < numberofLocalVariables; ++index)
  {
    SetLocalVariable(static_cast<uint16_t>(index), boost::intrusive_ptr<ObjectReference>(new ObjectReference(nullptr)));
  }

#ifdef _DEBUG
  JVMX_ASSERT(m_LocalVariableStack.size() == debugStackSize + debugNumberOfVariables);
  JVMX_ASSERT(debugNumberOfVariables == numberofLocalVariables);
#endif // _DEBUG


  if (nullptr != pLocalVariableTable)
  {
    for (size_t i = 0; i < pLocalVariableTable->GetNumberOfLocalVariables(); ++i)
    {
      ConstantPoolIndex index = pLocalVariableTable->GetIndexAt(i);
      ConstantPoolIndex descriptorIndex = pLocalVariableTable->GetDescriptorIndexAt(i);

      std::shared_ptr<ConstantPoolEntry> pConstant = pConstantPool->GetConstant(descriptorIndex);
      SetLocalVariable(index, TypeParser::GetDefaultValue(*pConstant->AsString()));
      NameLocalVariable(index, pLocalVariableTable->GetNameReferenceAt(i)->GetStringValue());
    }

    // Note: There might be an issue here if we need to reset the variables during the execution of a function.
    // It might not be a problem, but it might if a compiler assumes that the variable has the default value at some,
    // point. We should fix this in the execution engine so that for every instruction that is about to execute, we
    // need to check if a particular value needs to be re-initialised.
    // The JVM spec is not clear on whether or not this re-initialisation is required or not.
  }
}

void BasicVirtualMachineState::UpdateCurrentClassName(boost::intrusive_ptr<JavaString> pNewName)
{
  if (m_CurrentDisplayCallStackEntry.m_ClassName.IsEmpty())
  {
    throw InvalidStateException(__FUNCTION__ " - Tried to update the current class name when it already exists.");
  }

  UpdateCurrentClassName(*pNewName);
}

void BasicVirtualMachineState::UpdateCurrentClassName(JavaString newName)
{
  m_CurrentDisplayCallStackEntry.m_ClassName = newName;

  AssertValid();
}

void BasicVirtualMachineState::ReleaseLocalVariables()
{
  // Do nothing. This was done in PopState;
}

std::shared_ptr<MethodInfo> BasicVirtualMachineState::GetCurrentMethodInfo()
{
  AssertValid();

  if (m_MethodInfoStack.empty())
  {
    return nullptr;
  }

  return m_MethodInfoStack.back();
}

boost::intrusive_ptr<ObjectReference> BasicVirtualMachineState::CreateObject(std::shared_ptr<JavaClass> pClass)
{
  if (!pClass->IsInitialsed())
  {
    InitialiseClass(*pClass->GetName());
  }

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Creating instance of class: %s", pClass->GetName()->ToUtf8String().c_str());
  }
#endif // _DEBUG

  boost::intrusive_ptr<JavaString> pSuperClassName = pClass->GetSuperClassName();
  while (!pSuperClassName->IsEmpty())
  {
    std::shared_ptr<JavaClass> pSuperClass = LoadClass(*pSuperClassName);

    if (!pSuperClass->IsInitialsed())
    {
      InitialiseClass(*pSuperClassName);
    }

    pSuperClassName = pSuperClass->GetSuperClassName();
  }

  std::shared_ptr<IGarbageCollector> pGC = GlobalCatalog::GetInstance().Get("GarbageCollector");

  JavaObject* pObjectMemory = reinterpret_cast<JavaObject*>(pGC->AllocateObject(sizeof(JavaObject) + pClass->CalculateInstanceSizeInBytes()));
  JavaObject* pObject = new (pObjectMemory) JavaObject(pClass);

  std::shared_ptr<IObjectRegistry> pObjectRegistry = GlobalCatalog::GetInstance().Get("ObjectRegistry");
  boost::intrusive_ptr<ObjectReference> ref = new ObjectReference(pObjectRegistry->AddObject(pObject));
  pGC->AddRecentAllocation(ref);

#if defined(_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (HasUserCodeStarted())
  {
    GetLogger()->LogDebug("Completed creation of instance of class: %s", pClass->GetName()->ToUtf8String().c_str());
  }
#endif // _DEBUG

  return ref;
}

boost::intrusive_ptr<ObjectReference> BasicVirtualMachineState::CreateStringObject(const char* bytes)
{
  return CreateStringObject(JavaString::FromUtf8ByteArray(strlen(bytes), reinterpret_cast<const uint8_t*>(bytes)));
}

boost::intrusive_ptr<ObjectReference> BasicVirtualMachineState::CreateStringObject(const JavaString& string)
{
  static const JavaString c_StringClassName = JavaString::FromCString(JVMX_T("java/lang/String"));
  boost::intrusive_ptr<ObjectReference> pStringObject = CreateObject(GetClassLibrary()->FindClass(c_StringClassName));

#ifdef _DEBUG
  const size_t debugSize = GetOperandStackSize();
#endif // _DEBUG

  boost::intrusive_ptr<ObjectReference> pContents = CreateArray(e_JavaArrayTypes::Char, string.GetLengthInCodePoints());
  HelperTypes::ConvertJavaStringToArray(pContents, string);


  if (0 != string.GetLengthInCodePoints())
  {
    PushOperand(pStringObject);
    PushOperand(pContents); //[C
    PushOperand(boost::intrusive_ptr<JavaInteger>(new JavaInteger(JavaInteger::FromHostInt32(0)))); // I
    PushOperand(boost::intrusive_ptr<JavaInteger>(new JavaInteger(JavaInteger::FromHostInt32(pContents->GetContainedArray()->GetNumberOfElements())))); // I
    PushOperand(boost::intrusive_ptr<JavaBool>(new JavaBool(JavaBool::FromBool(true)))); // Z

    Execute(c_StringClassName, c_InstanceInitialisationMethodName, c_StringInitialisationMethodTypeWithArrayIntIntBool);
  }
  else
  {
    PushOperand(pStringObject);
    PushOperand(pContents);
    Execute(c_StringClassName, c_InstanceInitialisationMethodName, c_StringInitialisationMethodTypeWithArrayOnly);
  }

#ifdef _DEBUG
  JVMX_ASSERT(GetOperandStackSize() == debugSize);
#endif // _DEBUG

  return pStringObject;
}

std::shared_ptr<IClassLibrary> BasicVirtualMachineState::GetRuntimeConstantPool()
{
  return GetClassLibrary();
}

std::shared_ptr<MethodInfo> BasicVirtualMachineState::ResolveMethod(JavaClass* pClassFile, const JavaString& methodName, const JavaString& methodSignature)
{
  if (pClassFile->IsInterface())
  {
    std::shared_ptr<IExecutionEngine> pEngine = GlobalCatalog::GetInstance().Get("Engine");
    pEngine->ThrowJavaException(shared_from_this(), c_JavaIncompatibleClassChangeErrorException);
    return nullptr;
  }

  std::shared_ptr<MethodInfo> pMethodInfo = pClassFile->GetMethodByNameAndType(methodName, methodSignature);
  if (nullptr == pMethodInfo)
  {
    if (nullptr == pClassFile->GetSuperClass())
    {
      return nullptr;
    }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (HasUserCodeStarted())
    {
      GetLogger()->LogDebug(__FUNCTION__ " - Method %s with type %s *not* found on class %s. Trying Super class %s ", methodName.ToUtf8String().c_str(), methodSignature.ToUtf8String().c_str(), pClassFile->GetName()->ToUtf8String().c_str(), pClassFile->GetSuperClassName()->ToUtf8String().c_str());
    }
#endif // _DEBUG

    JavaClass* pSuperClass = pClassFile->GetSuperClass().get();
    if (nullptr == pSuperClass)
    {
      pSuperClass = pClassFile->GetSuperClass().get();
      if (nullptr == pSuperClass)
      {
        GetLogger()->LogError(__FUNCTION__ " - %s Could not load class file %s", GetCurrentClassAndMethodName(), pClassFile->GetSuperClass()->GetName()->ToUtf8String().c_str());

        // Assume Java Exception already thrown.
        return nullptr;
      }
    }

    pMethodInfo = ResolveMethod(pSuperClass, methodName, methodSignature);
  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (HasUserCodeStarted())
  {
    if (nullptr != pMethodInfo)
    {
      GetLogger()->LogDebug(__FUNCTION__ " - Method %s with type %s  found on class %s. ", methodName.ToUtf8String().c_str(), methodSignature.ToUtf8String().c_str(), pMethodInfo->GetClass()->GetName()->ToUtf8String().c_str());
    }
  }
#endif // _DEBUG

  return pMethodInfo;
}

std::shared_ptr<MethodInfo> BasicVirtualMachineState::ResolveMethodOnClass(boost::intrusive_ptr<JavaString> pClassName, const ConstantPoolMethodReference* pMethodRef)
{
  std::shared_ptr<JavaClass> pClassFile = GetClassLibrary()->FindClass(*pClassName);
  if (nullptr == pClassFile)
  {
    pClassFile = LoadClass(*pClassName);
    if (nullptr == pClassFile)
    {
      GetLogger()->LogError(__FUNCTION__ " - %s Could not load class file %s", GetCurrentClassAndMethodName(), pClassName->ToUtf8String().c_str());

      // Assume Java Exception already thrown.
      return nullptr;
    }
  }

  std::shared_ptr<MethodInfo> pMethodInfo = pClassFile->GetMethodByNameAndType(*pMethodRef->GetName(), *pMethodRef->GetType());
  if (nullptr == pMethodInfo)
  {
    if (nullptr == pClassFile->GetSuperClass())
    {
      return nullptr;
    }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (HasUserCodeStarted())
    {
      GetLogger()->LogDebug(__FUNCTION__ " - Method %s with type %s *not* found on class %s. Trying Super class %s ", pMethodRef->GetName()->ToUtf8String().c_str(), pMethodRef->GetType()->ToUtf8String().c_str(), pClassName->ToUtf8String().c_str(), pClassFile->GetSuperClassName()->ToUtf8String().c_str());
    }
#endif // _DEBUG
    pMethodInfo = ResolveMethodOnClass(pClassFile->GetSuperClass()->GetName(), pMethodRef);
  }

#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
  if (HasUserCodeStarted())
  {
    GetLogger()->LogDebug(__FUNCTION__ " - Method %s with type %s  found on class %s. ", pMethodRef->GetName()->ToUtf8String().c_str(), pMethodRef->GetType()->ToUtf8String().c_str(), pMethodInfo->GetClass()->GetName()->ToUtf8String().c_str());
  }
#endif // _DEBUG

  return pMethodInfo;
}

std::shared_ptr<JavaNativeInterface> BasicVirtualMachineState::GetJavaNativeInterface()
{
  return m_pJNI.lock();
}

void BasicVirtualMachineState::LogLocalVariables()
{
  AssertValid();

  GetLogger()->LogDebug("Dumping Local Variables (%d):", m_LocalVariableStackFramePointer);

  for (uint16_t i = 0; i < m_LocalVariableStack.size() - m_LocalVariableStackFramePointer; ++i)
  {
    auto pLocalVariable = GetLocalVariable(i);

    if (nullptr == pLocalVariable)
    {
      GetLogger()->LogDebug("NULL Local variable found. Bailing");
      return;
    }

    boost::intrusive_ptr<JavaString> pName = GetLocalVariableName(i);
    if (nullptr == pName)
    {
      GetLogger()->LogDebug("\t [%d] : %s", i, pLocalVariable->ToString().ToUtf8String().c_str());
    }
    else
    {
      GetLogger()->LogDebug("\t [%d][%s] : %s", i, pName->ToString().ToUtf8String().c_str(), pLocalVariable->ToString().ToUtf8String().c_str());
    }
  }
}

void BasicVirtualMachineState::InitialiseClass(const JavaString& className)
{
  std::shared_ptr<JavaClass> pClassFile = GetClassLibrary()->FindClass(className);

  if (nullptr == pClassFile)
  {
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (HasUserCodeStarted())
    {
      GetLogger()->LogDebug(__FUNCTION__ " - Class %s not loaded. Loading now.", className.ToUtf8String().c_str());
    }
#endif // defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)

    pClassFile = LoadClass(className);
    if (nullptr == pClassFile)
    {
#ifdef _DEBUG
      GetLogger()->LogDebug(__FUNCTION__ " - Returning because of load failure. Assume JavaException has been thrown.");
#endif // _DEBUG
      // We'd already have thrown a Java Exception.
      return;
    }
  }

  InitialiseClass(pClassFile);
}

void BasicVirtualMachineState::InitialiseClass(std::shared_ptr<JavaClass> pClassFile)
{
  std::lock_guard<std::recursive_mutex> pLock(pClassFile->GetInitialisationMutex());

  if (pClassFile->IsInitialsed())
  {
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
    if (HasUserCodeStarted())
    {
      GetLogger()->LogDebug(__FUNCTION__ " - Class %s already initialized.", (* pClassFile->GetName().get()).ToUtf8String().c_str());
    }
#endif // _DEBUG
    return;
  }

  // Recursively Load + Initialise Super Classes
  auto pSuperClass = pClassFile->GetSuperClass();
  if (nullptr != pSuperClass)
  {
    InitialiseClass(*pSuperClass->GetName());
  }

  pClassFile->SetInitialising();
  pClassFile->GetName();

  // Execute() will call PushState() and PopState()!!
  Execute(*pClassFile->GetName().get(), c_ClassInitialisationMethodName, c_ClassInitialisationMethodType);

  pClassFile->SetInitialised();

  return;
}

void BasicVirtualMachineState::LogCallStack()
{
  AssertValid();

  GetLogger()->LogDebug("Current Call Stack:");
  for (auto callStackEntry : m_DisplayCallStack)
  {
    GetLogger()->LogDebug("\t%s::%s%s - %lld", callStackEntry.m_ClassName.ToUtf8String().c_str(), callStackEntry.m_MethodName.ToUtf8String().c_str(), callStackEntry.m_MethodType.ToUtf8String().c_str(), callStackEntry.m_ProgramCounter);
  }

  GetLogger()->LogDebug("\t%s::%s%s - %lld", m_CurrentDisplayCallStackEntry.m_ClassName.ToUtf8String().c_str(), m_CurrentDisplayCallStackEntry.m_MethodName.ToUtf8String().c_str(), m_CurrentDisplayCallStackEntry.m_MethodType.ToUtf8String().c_str(), (int64_t)m_CurrentRegisters.m_ProgramCounter);
}

void BasicVirtualMachineState::LogOperandStack()
{
  AssertValid();

  GetLogger()->LogDebug("Current Operand Stack:");
  for (auto entry : m_OperandStack)
  {
    auto pOperand = entry.pOperand;
    std::string variableType = TypeParser::ConvertTypeToString(pOperand->GetVariableType()).ToUtf8String();
    std::string operandAsString = pOperand->ToString().ToUtf8String();

    GetLogger()->LogDebug("\t%s = %s", variableType.c_str(), operandAsString.c_str());
  }
}

boost::intrusive_ptr<ObjectReference> BasicVirtualMachineState::CreateAndInitialiseObject(std::shared_ptr<JavaClass> pClass)
{
  boost::intrusive_ptr<ObjectReference> pObject = CreateObject(pClass);

  std::shared_ptr<MethodInfo> pMethodInfo = pClass->GetMethodByNameAndType(c_InstanceInitialisationMethodName, c_InstanceInitialisationMethodType);
  if (nullptr == pMethodInfo)
  {
    std::shared_ptr<JavaClass> pSuperClass = pClass;
    pSuperClass = pSuperClass->GetSuperClass();
    while (nullptr != pSuperClass)
    {
      pMethodInfo = pSuperClass->GetMethodByNameAndType(c_InstanceInitialisationMethodName, c_InstanceInitialisationMethodType);
      if (nullptr != pMethodInfo)
      {
#if defined (_DEBUG) && defined(JVMX_LOG_VERBOSE)
        if (HasUserCodeStarted())
        {
          GetLogger()->LogDebug("Method found on Superclass of %s : Found method %s with type %s on superclass %s.", pClass->GetName()->ToUtf8String().c_str(), pMethodInfo->GetName()->ToUtf8String().c_str(), pMethodInfo->GetType()->ToUtf8String().c_str(), pMethodInfo->GetClass()->GetName()->ToUtf8String().c_str());
        }
#endif // _DEBUG
        break;
      }

      pSuperClass = pSuperClass->GetSuperClass();
    }
  }

  if (nullptr == pMethodInfo)
  {
    throw InvalidStateException(__FUNCTION__ " - Default constructor not found.");
  }

  std::vector<boost::intrusive_ptr<IJavaVariableType> > paramArray;
  PushState(*(pClass->GetName()), c_InstanceInitialisationMethodName, c_InstanceInitialisationMethodType, pMethodInfo);
  SetupLocalVariables(pMethodInfo, pObject, paramArray);
  Execute(*pMethodInfo);

  //   ReleaseLocalVariables();
  //   PopState();

  return pObject;
}

boost::intrusive_ptr<ObjectReference> BasicVirtualMachineState::CreateArray(e_JavaArrayTypes type, size_t size)
{
  return HelperTypes::CreateArray(type, size);
}

BasicVirtualMachineState::DisplayCallStackEntry::DisplayCallStackEntry()
  : m_ClassName(JavaString::EmptyString())
  , m_MethodName(JavaString::EmptyString())
  , m_MethodType(JavaString::EmptyString())
{}

std::vector<boost::intrusive_ptr<IJavaVariableType> > BasicVirtualMachineState::PopulateParameterArrayFromOperandStack(std::shared_ptr<MethodInfo> pMethodInfo)
{
  TypeParser::ParsedMethodType parsedType = TypeParser::ParseMethodType(*(pMethodInfo->GetType()));

  if (!pMethodInfo->IsStatic())
  {
    size_t arrayLength = parsedType.parameters.size();
    std::vector<boost::intrusive_ptr<IJavaVariableType> > paramArray(arrayLength + 1);

    for (size_t i = 0; i <= arrayLength; ++i)
    {
      paramArray[arrayLength - i] = PopOperand();
    }
    return paramArray;
  }
  else
  {
    size_t arrayLength = parsedType.parameters.size();
    std::vector<boost::intrusive_ptr<IJavaVariableType> > paramArray(arrayLength);

    for (size_t i = 0; i < arrayLength; ++i)
    {
      paramArray[(arrayLength - 1) - i] = PopOperand();
    }
    return paramArray;
  }
}

boost::intrusive_ptr<IJavaVariableType> BasicVirtualMachineState::PeekOperandFromBack(uint8_t count)
{
  uint8_t index = 1;
  for (auto it = m_OperandStack.rbegin(); it != m_OperandStack.rend(); ++it)
  {
    if (index == count)
    {
      return it->pOperand;
    }

    ++index;
  }

  return nullptr;
}

boost::intrusive_ptr<ObjectReference> BasicVirtualMachineState::CreateJavaLangClassFromClassName(const boost::intrusive_ptr<JavaString>& pClassName)
{
  return CreateJavaLangClassFromClassName(*pClassName);
}

boost::intrusive_ptr<ObjectReference> BasicVirtualMachineState::CreateJavaLangClassFromClassName(const JavaString& className)
{
  std::shared_ptr<IJavaLangClassList> pClassList = GlobalCatalog::GetInstance().Get("JavaLangClassList");

  boost::intrusive_ptr<ObjectReference> pResult = pClassList->Find(className);
  if (nullptr != pResult)
  {
    return pResult;
  }

  std::shared_ptr<JavaClass> pClassClass = GetClassLibrary()->FindClass(c_JavaLangClassName);
  if (nullptr == pClassClass)
  {
    throw InvalidStateException(__FUNCTION__ " - Expected to find class: java.lang.Class");
  }

  if (!pClassClass->IsInitialsed())
  {
    InitialiseClass(*pClassClass->GetName());
  }

  //pResult = CreateAndInitialiseObject( pClassClass );
  pResult = CreateObject(pClassClass);
  if (nullptr == pResult)
  {
    throw InvalidStateException(__FUNCTION__ " - Expected to create an instance of: java.lang.Class");
  }

  pResult->GetContainedObject()->SetJVMXField(c_SyntheticField_ClassName, boost::intrusive_ptr<JavaString>(new JavaString(className)));
  pClassList->Add(className, pResult);

  std::shared_ptr<MethodInfo> pMethodInfo = pResult->GetContainedObject()->GetClass()->GetMethodByNameAndType(c_InstanceInitialisationMethodName, c_JavaLangClassInitialisationMethodType);

  // Put result on the stack (This pointer for <init>)
  PushOperand(pResult);

  // Push null parameter
  PushOperand(boost::intrusive_ptr<ObjectReference>(new ObjectReference(nullptr)));

  ExecuteMethod(c_JavaLangClassName, c_InstanceInitialisationMethodName, c_JavaLangClassInitialisationMethodType, pMethodInfo);

  //pClassList->Add( *pClassName, pResult );

  return pResult;
}

void BasicVirtualMachineState::AssertValid() const
{
#ifdef _DEBUG
  JVMX_ASSERT(IsInitialRun() || !m_CurrentDisplayCallStackEntry.m_ClassName.IsEmpty());
#endif // _DEBUG
}

bool BasicVirtualMachineState::IsCurrentMethod(JavaString className, JavaString methodName, JavaString methodType) const
{
  return className == GetCurrentClassName() && methodName == GetCurrentMethodName() && methodType == GetCurrentMethodType();
}

boost::intrusive_ptr<ObjectReference> BasicVirtualMachineState::FindJavaLangClass(const JavaString& className) const
{
  AssertValid();
  std::shared_ptr<IJavaLangClassList> pClassList = GlobalCatalog::GetInstance().Get("JavaLangClassList");
  return pClassList->Find(className);
}

void BasicVirtualMachineState::ExecuteMethod(const JavaString& className, const JavaString& methodName, const JavaString& methodType, std::shared_ptr<MethodInfo> pInitialMethod)
{
  // #ifdef _DEBUG
  //   uint32_t displayStackDepth = m_DisplayCallStack.size();
  // #endif // _DEBUG

  PushState(className, methodName, methodType, pInitialMethod);

  DoSynchronisation(pInitialMethod);

  SetupLocalVariables(pInitialMethod);

#ifdef _DEBUG
  AssertValid();
#endif // _DEBUG

  Execute(*pInitialMethod);

  // #ifdef _DEBUG
  //   AssertValid();
  //   JVMX_ASSERT( m_DisplayCallStack.size() == displayStackDepth );
  // #endif // _DEBUG
}

void BasicVirtualMachineState::DoSynchronisation(std::shared_ptr<MethodInfo> pInitialMethod)
{
  if (pInitialMethod->IsSynchronised())
  {
    if (pInitialMethod->IsStatic())
    {
      PushMonitor(pInitialMethod->GetClass()->MonitorEnter(GetCurrentClassAndMethodName().ToUtf8String().c_str()));
    }
    else
    {
      boost::intrusive_ptr<IJavaVariableType> pTopOperand = PeekOperand();
      if (e_JavaVariableTypes::Object == pTopOperand->GetVariableType())
      {
        boost::intrusive_ptr<ObjectReference> pObject = boost::dynamic_pointer_cast<ObjectReference>(pTopOperand);
        PushMonitor(pObject->GetContainedObject()->MonitorEnter(GetCurrentClassAndMethodName().ToUtf8String().c_str()));
      }
      else if (e_JavaVariableTypes::Array == pTopOperand->GetVariableType())
      {
        boost::intrusive_ptr<ObjectReference> pArray = boost::dynamic_pointer_cast<ObjectReference>(pTopOperand);
        PushMonitor(pArray->GetContainedArray()->MonitorEnter(GetCurrentClassAndMethodName().ToUtf8String().c_str()));
      }
      else
      {
        throw InvalidStateException(__FUNCTION__ " - Expected an array or an object on top of the operand stack.");
      }
    }
  }
}

size_t BasicVirtualMachineState::CalculateNumberOfStackItemsToClear(size_t exceptionRegionStart) const
{
  size_t numberOfItemsToClear = 0;

  for (auto it = m_OperandStack.crbegin(); it != m_OperandStack.crend(); ++it)
  {
    bool isOperandInExceptionScope = it->allocatedProgramCount >= exceptionRegionStart;

    // This implementation does *not* guard against recursive methods that throw exceptions. It will most probably
    // do the wrong thing in that case.
    // if ( !IsCurrentMethod( JavaString::FromCString( (const char16_t *)it->m_pClassName ), JavaString::FromCString( (const char16_t *)it->m_MethodName), JavaString::FromCString( (const char16_t *)it->m_MethodType) )it->m_MethodType ) )
    if (!IsCurrentMethod(it->m_pClassName, it->m_MethodName, it->m_MethodType)
      || !isOperandInExceptionScope)
    {
      break;
    }

    ++numberOfItemsToClear;
  }

  return numberOfItemsToClear;
}

void BasicVirtualMachineState::DoGarbageCollection()
{
  std::shared_ptr<IGarbageCollector> pGC = GlobalCatalog::GetInstance().Get("GarbageCollector");
  pGC->Collect( /* e_ForceGarbageCollection::No, shared_from_this() */);
}

void BasicVirtualMachineState::StartShutdown(int exitCode)
{
  if (IsShuttingDown())
  {
    return;
  }

  m_isShuttingDown = true;

  PushOperand(boost::intrusive_ptr<JavaInteger>(new JavaInteger(JavaInteger::FromHostInt32(exitCode))));
  Execute(JavaString::FromCString("java/lang/System"), JavaString::FromCString("exit"), JavaString::FromCString("(I)V"));
}

void BasicVirtualMachineState::Halt(int exitCode)
{
  std::shared_ptr<IExecutionEngine> pEngine = GlobalCatalog::GetInstance().Get("ExecutionEngine");
  pEngine->Halt();
  m_ExitCode = exitCode;
}

boost::intrusive_ptr<ObjectReference> BasicVirtualMachineState::GetClassLoaderForClassObject(boost::intrusive_ptr<ObjectReference> pObject)
{
  boost::intrusive_ptr<JavaString> pClassName = boost::dynamic_pointer_cast<JavaString>(pObject->GetContainedObject()->GetJVMXFieldByName(c_SyntheticField_ClassName));
  std::shared_ptr<JavaClass> pClass = LoadClass(*pClassName);

  return pClass->GetClassLoader();
}

std::vector<boost::intrusive_ptr<IJavaVariableType>> BasicVirtualMachineState::GetGCRoots() const
{
  std::vector<boost::intrusive_ptr<IJavaVariableType>> roots;

  for (auto entry : m_OperandStack)
  {
    if (e_JavaVariableTypes::Object == entry.pOperand->GetVariableType() ||
      e_JavaVariableTypes::Array == entry.pOperand->GetVariableType())
    {
      roots.push_back(entry.pOperand);
    }
  }

  for (auto entry : m_LocalVariableStack)
  {
    if (e_JavaVariableTypes::Object == entry.m_pValue->GetVariableType() ||
      e_JavaVariableTypes::Array == entry.m_pValue->GetVariableType())
    {
      roots.push_back(entry.m_pValue);
    }
  }

  for (auto entry : m_GlobalReferences)
  {
    if (e_JavaVariableTypes::Object == entry->GetVariableType() ||
      e_JavaVariableTypes::Array == entry->GetVariableType())
    {
      roots.push_back(entry);
    }
  }

  for (auto frame : m_LocalReferenceFrames)
  {
    for (auto entry : frame)
    {
      if (e_JavaVariableTypes::Object == entry->GetVariableType() ||
        e_JavaVariableTypes::Array == entry->GetVariableType())
      {
        roots.push_back(entry);
      }
    }
  }

  if (m_ExceptionOccurred)
  {
    roots.push_back(m_pException);
  }

  return roots;
}

std::vector<boost::intrusive_ptr<IJavaVariableType>> BasicVirtualMachineState::GetStaticObjectsAndArrays() const
{
  return m_pClassLibrary->GetAllStaticObjectsAndArrays();
}


void BasicVirtualMachineState::Pause()
{
  JVMX_ASSERT(!m_isPaused && !m_isPausing);
  m_isPausing = true;
  if (m_NativeExecutionCount)
  {
    m_isPaused = true;
  }
}

void BasicVirtualMachineState::Resume()
{
  //JVMX_ASSERT( m_isPaused );
  m_isPausing = false;
  m_isPaused = false;
}

bool BasicVirtualMachineState::IsPaused() const
{
  return m_isPaused;
}

bool BasicVirtualMachineState::IsPausing() const
{
  return m_isPausing;
}

void BasicVirtualMachineState::ConfirmPaused()
{
  m_isPaused = true;
  m_isPausing = false;
}

std::atomic_int64_t& BasicVirtualMachineState::GetStackLevel()
{
  return m_StackLevel;
}

bool BasicVirtualMachineState::GetInterruptedFlag()
{
  return m_isInterrupted;
}

void BasicVirtualMachineState::Interrupt()
{
  m_isInterrupted = true;
}

void BasicVirtualMachineState::AddGlobalReference(boost::intrusive_ptr<ObjectReference> pObject)
{
  m_GlobalReferences.push_back(pObject);
}

void BasicVirtualMachineState::DeleteGlobalReference(boost::intrusive_ptr<ObjectReference> pObject)
{
  m_GlobalReferences.remove(pObject);
}

void BasicVirtualMachineState::AddLocalReference(boost::intrusive_ptr<ObjectReference> pObject)
{
  JVMX_ASSERT(!m_LocalReferenceFrames.empty());
  m_LocalReferenceFrames.back().push_back(pObject);
}

void BasicVirtualMachineState::DeleteLocalReference(boost::intrusive_ptr<ObjectReference> pObject)
{
  JVMX_ASSERT(!m_LocalReferenceFrames.empty());

  auto list = m_LocalReferenceFrames.back();

  for (auto i = list.begin(); i != list.end(); ++i)
  {
    if ((*i)->GetIndex() == pObject->GetIndex())
    {
      list.erase(i);
      return;
    }
  }

  JVMX_ASSERT(false);
}

void BasicVirtualMachineState::AddLocalReferenceFrame()
{
  m_LocalReferenceFrames.push_back(std::list<boost::intrusive_ptr<ObjectReference>>());
}

void BasicVirtualMachineState::DeleteLocalReferenceFrame()
{
  JVMX_ASSERT(!m_LocalReferenceFrames.empty());
  m_LocalReferenceFrames.pop_back();
}

void BasicVirtualMachineState::SetExecutingNative()
{
  JVMX_ASSERT(m_NativeExecutionCount >= 0);
  ++m_NativeExecutionCount;
}

void BasicVirtualMachineState::SetExecutingHosted()
{
  --m_NativeExecutionCount;
  JVMX_ASSERT(m_NativeExecutionCount >= 0);
}

bool BasicVirtualMachineState::IsExecutingNative() const
{
  return m_NativeExecutionCount > 0;
}

void BasicVirtualMachineState::SetUserCodeStarted()
{
  m_hasUserCodeStarted = true;
}

bool BasicVirtualMachineState::HasUserCodeStarted() const
{
  return m_hasUserCodeStarted;
}

#ifdef _DEBUG
size_t BasicVirtualMachineState::GetOperandStackSize()
{
  return m_OperandStack.size();
}
#endif // _DEBUG