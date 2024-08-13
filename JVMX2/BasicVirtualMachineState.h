#ifndef _BASICVIRTUALMACHINESTATE__H_
#define _BASICVIRTUALMACHINESTATE__H_

#include <stack>
#include <list>
#include <map>

#include <wallaroo/collaborator.h>

#include "CodeAttributeLocalVariableTable.h"
#include "DefaultJavaLangClassList.h"
#include "MethodInfo.h"

#include "IExecutionEngine.h"
#include "IVirtualMachineState.h"
#include "JVMRegisters.h"

class JavaNativeInterface;

class BasicVirtualMachineState : public IVirtualMachineState
{
public:
  BasicVirtualMachineState( std::shared_ptr<VirtualMachine> pVM, bool hasUserCodeStarted = false);
  virtual ~BasicVirtualMachineState() JVMX_NOEXCEPT;

  virtual std::shared_ptr<IVirtualMachineState> CreateNewState() JVMX_OVERRIDE;

  virtual void Execute( const JavaString &startingClassName, const JavaString &methodName, const JavaString &methodType ) JVMX_OVERRIDE;
  virtual void Execute( const MethodInfo &method ) JVMX_OVERRIDE;

  //virtual std::shared_ptr<IMemoryManager> GetHeap() const JVMX_OVERRIDE;
  virtual std::shared_ptr<ILogger> GetLogger() JVMX_OVERRIDE;
  virtual std::shared_ptr<IClassLibrary> GetClassLibrary();

  virtual std::shared_ptr<MethodInfo> GetMethod( size_t index ) JVMX_OVERRIDE;
  //virtual const FieldInfo &GetField( size_t index ) const JVMX_OVERRIDE;

  virtual uintptr_t GetProgramCounter() const JVMX_OVERRIDE;
  virtual void AdvanceProgramCounter( int byteCount ) JVMX_OVERRIDE;

  virtual size_t GetCodeSegmentLength() const JVMX_OVERRIDE;
  virtual const uint8_t *GetCodeSegmentStart() const JVMX_OVERRIDE;

  virtual bool CanReadBytes( int byteCount ) const JVMX_OVERRIDE;

  virtual std::shared_ptr<ConstantPoolEntry> GetConstantFromCurrentClass( ConstantPoolIndex index ) JVMX_OVERRIDE;

  virtual bool IsClassInitialised( const JavaString &className ) JVMX_OVERRIDE;
  virtual std::shared_ptr<JavaClass> GetCurrentClass()  JVMX_OVERRIDE;

  virtual const JavaString &GetCurrentClassName() const JVMX_OVERRIDE;
  virtual const JavaString &GetCurrentMethodName() const JVMX_OVERRIDE;
  virtual const JavaString &GetCurrentMethodType() const JVMX_OVERRIDE;

  virtual std::shared_ptr<MethodInfo> GetMethodByNameAndType( const JavaString &className, const JavaString &methodName, const JavaString &methodType, std::shared_ptr<IClassLibrary> pConstantPool ) JVMX_OVERRIDE;
  virtual std::shared_ptr<JavaClass> GetClassByName( std::shared_ptr<IClassLibrary> pConstantPool, const JavaString &className );

  virtual void PushState( const JavaString &newClassName, const JavaString &newMethodName, const JavaString &newMethodType, std::shared_ptr<MethodInfo> pNewMethodInfo ) JVMX_OVERRIDE;
  virtual void PopState() JVMX_OVERRIDE;

  virtual const CodeAttributeStackMapTable *GetCurrentStackMap() JVMX_OVERRIDE;
  virtual const ClassAttributeCode *GetCurrentCodeInfo() JVMX_OVERRIDE;

  virtual const JavaString &GetCurrentClassAndMethodName() const JVMX_OVERRIDE;
  virtual void SetCodeSegment( const ClassAttributeCode *codeInfo ) JVMX_OVERRIDE;

  virtual void PushOperand( const boost::intrusive_ptr<IJavaVariableType> &pOperand ) JVMX_OVERRIDE;
  virtual boost::intrusive_ptr<IJavaVariableType> PopOperand() JVMX_OVERRIDE;
  virtual boost::intrusive_ptr<IJavaVariableType> PeekOperand() JVMX_OVERRIDE;
  virtual boost::intrusive_ptr<IJavaVariableType> PeekOperandFromBack( uint8_t count ) JVMX_OVERRIDE;

  virtual boost::intrusive_ptr<ObjectReference> SetupLocalVariables( std::shared_ptr<MethodInfo> pMethodInfo ) JVMX_OVERRIDE;
  virtual void SetupLocalVariables( std::shared_ptr<MethodInfo> pMethodInfo, boost::intrusive_ptr<ObjectReference> pObject, const std::vector<boost::intrusive_ptr<IJavaVariableType> > &paramArray ) JVMX_OVERRIDE;

  void SetupLocalVariableTypeReference( const std::vector<boost::intrusive_ptr<IJavaVariableType> > &paramArray, size_t paramIndex, uint16_t localVariableIndex );

  virtual boost::intrusive_ptr<IJavaVariableType> GetLocalVariable( uint16_t localVariableIndex ) JVMX_OVERRIDE;

  virtual const boost::intrusive_ptr<JavaString> &GetLocalVariableName( uint16_t localVariableIndex );

  virtual void SetLocalVariable( uint16_t localVariableIndex, boost::intrusive_ptr<IJavaVariableType> pValue ) JVMX_OVERRIDE;
  virtual void SetLocalVariable( uint16_t localVariableIndex, IJavaVariableType *pValue ) JVMX_FN_DELETE;


  virtual void NameLocalVariable( uint16_t localVariableIndex, boost::intrusive_ptr<JavaString> pName );

  virtual void UpdateCurrentClassName( boost::intrusive_ptr<JavaString> pNewName ) JVMX_OVERRIDE;
  virtual void UpdateCurrentClassName( JavaString pNewName );

  virtual std::shared_ptr<JavaClass> LoadClass( const JavaString &className, const JavaString &path = JavaString::EmptyString() ) JVMX_OVERRIDE;
  virtual std::shared_ptr<JavaClass> LoadClass(const DataBuffer& classData) JVMX_OVERRIDE;


  virtual void ReleaseLocalVariables() JVMX_OVERRIDE;

  virtual std::shared_ptr<MethodInfo> GetCurrentMethodInfo() JVMX_OVERRIDE;

  virtual boost::intrusive_ptr<ObjectReference> CreateStringObject( const char *bytes ) JVMX_OVERRIDE;
  virtual boost::intrusive_ptr<ObjectReference> CreateStringObject( const JavaString &string ) JVMX_OVERRIDE;

  virtual boost::intrusive_ptr<ObjectReference> CreateObject( std::shared_ptr<JavaClass> pClass ) JVMX_OVERRIDE;
  __declspec( deprecated ) virtual boost::intrusive_ptr<ObjectReference> CreateAndInitialiseObject( std::shared_ptr<JavaClass> pClass ) JVMX_OVERRIDE;

  virtual boost::intrusive_ptr<ObjectReference> CreateArray( e_JavaArrayTypes type, size_t size ) JVMX_OVERRIDE;

  virtual void InitialiseClass( const JavaString &className ) JVMX_OVERRIDE;
  virtual void InitialiseClass(std::shared_ptr<JavaClass> pClass) JVMX_OVERRIDE;

  virtual std::shared_ptr<IClassLibrary> GetRuntimeConstantPool() JVMX_OVERRIDE;
  virtual std::shared_ptr<JavaNativeInterface> GetJavaNativeInterface() JVMX_OVERRIDE;

  virtual std::shared_ptr<MethodInfo> ResolveMethod( JavaClass *pClass, const JavaString &methodName, const JavaString &methodSignature ) JVMX_OVERRIDE;
  virtual std::shared_ptr<MethodInfo> ResolveMethodOnClass( boost::intrusive_ptr<JavaString> pClassName, const ConstantPoolMethodReference *pMethodRef ) JVMX_OVERRIDE;

  virtual std::vector<boost::intrusive_ptr<IJavaVariableType> > PopulateParameterArrayFromOperandStack( std::shared_ptr<MethodInfo> pMethodInfo ) JVMX_OVERRIDE;

  virtual void LogLocalVariables() JVMX_OVERRIDE;
  virtual void LogCallStack() JVMX_OVERRIDE;
  virtual void LogOperandStack() JVMX_OVERRIDE;

  virtual boost::intrusive_ptr<ObjectReference> FindJavaLangClass( const JavaString &className ) const JVMX_OVERRIDE;
  virtual boost::intrusive_ptr<ObjectReference> CreateJavaLangClassFromClassName( const boost::intrusive_ptr<JavaString> &pClassName ) JVMX_OVERRIDE;
  virtual boost::intrusive_ptr<ObjectReference> CreateJavaLangClassFromClassName( const JavaString &className ) JVMX_OVERRIDE;

  virtual void ExecuteMethod( const JavaString &className, const JavaString &methodName, const JavaString &methodType, std::shared_ptr<MethodInfo> pInitialMethod ) JVMX_OVERRIDE;

  void DoSynchronisation( std::shared_ptr<MethodInfo> pInitialMethod );

  virtual size_t CalculateNumberOfStackItemsToClear( size_t exceptionRegionStart ) const JVMX_OVERRIDE;

  virtual void DoGarbageCollection() JVMX_OVERRIDE;

  virtual void StartShutdown( int exitCode ) JVMX_OVERRIDE;
  virtual void Halt( int exitCode ) JVMX_OVERRIDE;

  virtual bool IsShuttingDown() const JVMX_OVERRIDE;

  virtual int GetExitCode() const JVMX_OVERRIDE;

  virtual void RunAllFinalizers() JVMX_OVERRIDE;

  //virtual std::shared_ptr<VirtualMachine> GetVM() JVMX_OVERRIDE;

  virtual void SetExceptionThrown( boost::intrusive_ptr<ObjectReference> pException ) JVMX_OVERRIDE;
  virtual bool HasExceptionOccurred() const JVMX_OVERRIDE;
  virtual boost::intrusive_ptr<ObjectReference> GetException() JVMX_OVERRIDE;
  virtual void ResetException() JVMX_OVERRIDE;

  virtual void PushMonitor( std::shared_ptr<Lockable> pMutex ) JVMX_OVERRIDE;
  virtual std::shared_ptr<Lockable> PopMonitor() JVMX_OVERRIDE;

  virtual void PushAndZeroCallStackDepth() JVMX_OVERRIDE;
  virtual uint16_t PopCallStackDepth() JVMX_OVERRIDE;
  virtual uint16_t GetCallStackDepth() JVMX_OVERRIDE;
  virtual uint16_t IncrementCallStackDepth() JVMX_OVERRIDE;
  virtual uint16_t DecrementCallStackDepth() JVMX_OVERRIDE;

  virtual void SetJavaNativeInterface( std::weak_ptr<JavaNativeInterface> pJNI ) JVMX_OVERRIDE;

  virtual ThreadInfo ReturnCurrentThreadInfo() JVMX_OVERRIDE;
  virtual void RegisterNativeMethods( std::shared_ptr<JavaNativeInterface> pJNI ) JVMX_OVERRIDE;
  virtual void AddThread( std::shared_ptr<boost::thread> pNewThread, boost::intrusive_ptr<ObjectReference> pObject, const std::shared_ptr<IVirtualMachineState> &pNewState ) JVMX_OVERRIDE;

  virtual boost::intrusive_ptr<ObjectReference> GetCallStackOfClassObjects() JVMX_OVERRIDE;
  virtual boost::intrusive_ptr<ObjectReference> GetCallStackOfStackTraceElements() JVMX_OVERRIDE;

  virtual boost::intrusive_ptr<ObjectReference> GetClassLoaderForClassObject( boost::intrusive_ptr<ObjectReference> pObject ) JVMX_OVERRIDE;
  virtual boost::intrusive_ptr<ObjectReference> GetClassLoaderForClassObject( const IJavaVariableType *pObject ) JVMX_FN_DELETE;


  virtual std::vector<boost::intrusive_ptr<IJavaVariableType>> GetGCRoots() const JVMX_OVERRIDE;
  virtual std::vector<boost::intrusive_ptr<IJavaVariableType>> GetStaticObjectsAndArrays() const JVMX_OVERRIDE;

  virtual void Pause() JVMX_OVERRIDE;
  virtual void Resume() JVMX_OVERRIDE;
  virtual bool IsPaused() const JVMX_OVERRIDE;
  virtual bool IsPausing() const JVMX_OVERRIDE;

  // Do not call. Used by the Execution Engine.
  virtual void ConfirmPaused() JVMX_OVERRIDE;

  virtual std::atomic_int64_t &GetStackLevel() JVMX_OVERRIDE;

  //virtual bool GetAndClearInterruptedFlag() JVMX_OVERRIDE;
  virtual bool GetInterruptedFlag() JVMX_OVERRIDE;
  virtual void Interrupt() JVMX_OVERRIDE;

  virtual void AddGlobalReference( boost::intrusive_ptr<ObjectReference> pObject ) JVMX_OVERRIDE;
  virtual void DeleteGlobalReference( boost::intrusive_ptr<ObjectReference> pObject ) JVMX_OVERRIDE;

  virtual void AddLocalReferenceFrame() JVMX_OVERRIDE;
  virtual void DeleteLocalReferenceFrame() JVMX_OVERRIDE;

  virtual void AddLocalReference( boost::intrusive_ptr<ObjectReference> pObject ) JVMX_OVERRIDE;
  virtual void DeleteLocalReference( boost::intrusive_ptr<ObjectReference> pObject ) JVMX_OVERRIDE;

  virtual void SetExecutingNative() JVMX_OVERRIDE;
  virtual void SetExecutingHosted() JVMX_OVERRIDE;
  virtual bool IsExecutingNative() const JVMX_OVERRIDE;

  virtual void SetUserCodeStarted() JVMX_OVERRIDE;
  virtual bool HasUserCodeStarted() const JVMX_OVERRIDE;

#ifdef _DEBUG
  virtual size_t GetOperandStackSize() JVMX_OVERRIDE;
#endif // _DEBUG

private:

  virtual void PushOperand( const IJavaVariableType *pOperand ) JVMX_FN_DELETE;
  virtual void PushOperand( IJavaVariableType *pOperand ) JVMX_FN_DELETE;

  ClassAttributeCode FindCodeAttribute( const MethodInfo &method );

  void PushMethodStack( const JavaString &newClassName, const JavaString &newMethodName, const JavaString &newMethodType );
  void PopMethodStack();

  bool IsInitialRun() const JVMX_NOEXCEPT;

  //size_t CalculateNumberOfParameters( const JavaString &type );

  void InitialiseLocalVariables( size_t numberofLocalVarialbes, std::shared_ptr<CodeAttributeLocalVariableTable> pLocalVariableTable, std::shared_ptr<ConstantPool> pConstantPool );

  //void AddFieldsToObject( std::shared_ptr<JavaClass> pClass, boost::intrusive_ptr<ObjectReference> pObject );

  void AssertValid() const;
  bool IsCurrentMethod( JavaString className, JavaString methodName, JavaString methodType ) const;

  void ReleaseMemory();

  static std::shared_ptr<CodeAttributeLocalVariableTable> GetLocalVariableTable( std::shared_ptr<MethodInfo> pMethodInfo );
  JavaString BuildCurrentClassAndMethodName() const;

  const boost::intrusive_ptr<JavaString> GetSoureFileName( const std::shared_ptr<MethodInfo> pMethodInfo ) const;
  uint16_t GetLineNumber( int stackPos );

private:
  std::shared_ptr<ILogger> m_pLogger;
  std::shared_ptr<IClassLibrary> m_pClassLibrary;
  std::shared_ptr<VirtualMachine> m_pVM;

  size_t m_LocalVariableStackFramePointer;
  std::stack<size_t> m_LocalVariableStackFrameStack;

  bool m_isShuttingDown;

private:

  struct LocalVariableEntry
  {
    boost::intrusive_ptr<IJavaVariableType> m_pValue;
    boost::intrusive_ptr<JavaString> m_pName;
  };

  std::vector<LocalVariableEntry> m_LocalVariableStack;

private:
  struct OperandStackEntry
  {
    boost::intrusive_ptr<IJavaVariableType> pOperand;
    size_t allocatedProgramCount;
    size_t m_callStackDepth;

    JavaString m_pClassName;
    JavaString m_MethodName;
    JavaString m_MethodType;
  };
  std::list< OperandStackEntry > m_OperandStack;

private:
  struct DisplayCallStackEntry
  {
    DisplayCallStackEntry();

    JavaString m_ClassName;
    JavaString m_MethodName;
    JavaString m_MethodType;
    int64_t m_ProgramCounter;
  };

  std::list<DisplayCallStackEntry> m_DisplayCallStack;

  DisplayCallStackEntry m_CurrentDisplayCallStackEntry;

private:
  std::vector<JVMRegisters> m_RegisterStack;
  std::vector< std::shared_ptr<MethodInfo> > m_MethodInfoStack;
  std::stack< std::shared_ptr<Lockable> > m_MutexStack;
  std::weak_ptr<JavaNativeInterface> m_pJNI;
  JavaString m_CurrentClassAndMethodName;

  JVMRegisters m_CurrentRegisters;

  int m_ExitCode;

  volatile bool m_isPaused;
  volatile bool m_isPausing;
  int m_NativeExecutionCount;
  bool m_hasUserCodeStarted;

  bool m_isInterrupted;

  bool m_ExceptionOccurred;
  boost::intrusive_ptr<ObjectReference> m_pException;

  uint16_t m_CallStackDepth;
  std::stack<uint16_t> m_CallStackDepthStack;


  std::atomic_int64_t m_StackLevel;
  std::list<boost::intrusive_ptr<ObjectReference>> m_GlobalReferences;

  std::list< std::list<boost::intrusive_ptr<ObjectReference>> > m_LocalReferenceFrames;

#if _DEBUG
  unsigned long m_ThreadId;
#endif

};

#endif // _BASICVIRTUALMACHINESTATE__H_
