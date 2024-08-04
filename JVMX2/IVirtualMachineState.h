#ifndef _IVIRTUALMACHINESTATE__H_
#define _IVIRTUALMACHINESTATE__H_

#include <memory>
#include <vector>

#include "GlobalConstants.h"
#include "ThreadManager.h"
#include "ThreadInfo.h"

#include "MethodInfo.h"
#include "JavaTypes.h"
#include "Lockable.h"

// Forward Declarations
class IMemoryManager;
class IStackManager;
class ILogger;
class IClassLibrary;
class MethodInfo;
class FieldInfo;
class ConstantPoolEntry;
class JavaClass;
class JavaClassReference;
class VirtualMachine;
class JavaNativeInterface;

class IVirtualMachineState : public std::enable_shared_from_this<IVirtualMachineState>
{
public:
  virtual ~IVirtualMachineState() JVMX_NOEXCEPT {};

  virtual std::shared_ptr<IVirtualMachineState> CreateNewState() JVMX_PURE;

  virtual uintptr_t GetProgramCounter() const JVMX_PURE;

  virtual void Execute( const JavaString &startingClassName, const JavaString &methodName, const JavaString &methodType ) JVMX_PURE;
  virtual void Execute( const MethodInfo &method ) JVMX_PURE;

  virtual std::shared_ptr<ILogger> GetLogger() JVMX_PURE;

  virtual std::shared_ptr<MethodInfo> GetMethod( size_t index ) JVMX_PURE;
  //virtual const FieldInfo &GetField( size_t index ) const JVMX_PURE;

  virtual void AdvanceProgramCounter( int byteCount ) JVMX_PURE;

  virtual const uint8_t *GetCodeSegmentStart() const JVMX_PURE;
  virtual size_t GetCodeSegmentLength() const JVMX_PURE;

  virtual bool CanReadBytes( int byteCount ) const JVMX_PURE;

  virtual std::shared_ptr<ConstantPoolEntry> GetConstantFromCurrentClass( ConstantPoolIndex index ) JVMX_PURE;
  //virtual ConstantPoolEntry GetConstantFromOtherClass( const JavaString &className, ConstantPoolIndex index ) const JVMX_PURE;

  virtual const CodeAttributeStackMapTable *GetCurrentStackMap() JVMX_PURE;
  virtual const ClassAttributeCode *GetCurrentCodeInfo() JVMX_PURE;

  virtual bool IsClassInitialised( const JavaString &className ) JVMX_PURE;

  virtual std::shared_ptr<JavaClass> GetCurrentClass() JVMX_PURE;

  virtual std::shared_ptr<MethodInfo> GetMethodByNameAndType( const JavaString &className, const JavaString &methodName, const JavaString &methodType, std::shared_ptr<IClassLibrary> pConstantPool ) JVMX_PURE;

  virtual void PushState( const JavaString &newClassName, const JavaString &newMethodName, const JavaString &newMethodType, std::shared_ptr<MethodInfo> pNewMethodInfo ) JVMX_PURE;
  virtual void PopState() JVMX_PURE;

  virtual const JavaString &GetCurrentClassAndMethodName() const JVMX_PURE;
  virtual const JavaString &GetCurrentClassName() const JVMX_PURE;
  virtual const JavaString &GetCurrentMethodName() const JVMX_PURE;
  virtual const JavaString &GetCurrentMethodType() const JVMX_PURE;

  virtual void SetCodeSegment( const ClassAttributeCode *codeInfo ) JVMX_PURE;

  virtual void PushOperand( const boost::intrusive_ptr<IJavaVariableType> &pOperand ) JVMX_PURE;
  virtual boost::intrusive_ptr<IJavaVariableType> PopOperand() JVMX_PURE;
  virtual boost::intrusive_ptr<IJavaVariableType> PeekOperand() JVMX_PURE;
  virtual boost::intrusive_ptr<IJavaVariableType> PeekOperandFromBack( uint8_t count ) JVMX_PURE;

  virtual boost::intrusive_ptr<ObjectReference> SetupLocalVariables( std::shared_ptr<MethodInfo> pMethodInfo ) JVMX_PURE;
  virtual void SetupLocalVariables( std::shared_ptr<MethodInfo> pMethodInfo, boost::intrusive_ptr<ObjectReference> pObject, const std::vector<boost::intrusive_ptr<IJavaVariableType> > &paramArray ) JVMX_PURE;

  virtual boost::intrusive_ptr<IJavaVariableType> GetLocalVariable( uint16_t localVariableIndex ) JVMX_PURE;
  virtual void SetLocalVariable( uint16_t localVariableIndex, boost::intrusive_ptr<IJavaVariableType> pValue ) JVMX_PURE;

  virtual std::shared_ptr<JavaClass> LoadClass( const JavaString &className, const JavaString &path = JavaString::EmptyString() ) JVMX_PURE;

  virtual void UpdateCurrentClassName( boost::intrusive_ptr<JavaString> pNewName ) JVMX_PURE;

  virtual void ReleaseLocalVariables() JVMX_PURE;

  virtual std::shared_ptr<MethodInfo> GetCurrentMethodInfo() JVMX_PURE;

  virtual boost::intrusive_ptr<ObjectReference> CreateStringObject( const char *bytes ) JVMX_PURE;
  virtual boost::intrusive_ptr<ObjectReference> CreateStringObject( const JavaString &string ) JVMX_PURE;
  virtual boost::intrusive_ptr<ObjectReference> CreateObject( std::shared_ptr<JavaClass> pClass ) JVMX_PURE;
  __declspec( deprecated ) virtual boost::intrusive_ptr<ObjectReference> CreateAndInitialiseObject( std::shared_ptr<JavaClass> pClass ) JVMX_PURE;

  virtual boost::intrusive_ptr<ObjectReference> CreateArray( e_JavaArrayTypes type, size_t size ) JVMX_PURE;

  virtual std::shared_ptr<IClassLibrary> GetRuntimeConstantPool() JVMX_PURE;

  virtual std::shared_ptr<MethodInfo> ResolveMethod( JavaClass *pClass, const JavaString &methodName, const JavaString &methodSignature ) JVMX_PURE;

  virtual std::shared_ptr<MethodInfo> ResolveMethodOnClass( boost::intrusive_ptr<JavaString> pClassName, const ConstantPoolMethodReference *pMethodRef ) JVMX_PURE;

  virtual std::shared_ptr<JavaNativeInterface> GetJavaNativeInterface() JVMX_PURE;

  virtual void LogLocalVariables() JVMX_PURE;
  virtual void LogCallStack() JVMX_PURE;
  virtual void LogOperandStack() JVMX_PURE;

  virtual void InitialiseClass( const JavaString &className ) JVMX_PURE;

  virtual std::vector<boost::intrusive_ptr<IJavaVariableType> > PopulateParameterArrayFromOperandStack( std::shared_ptr<MethodInfo> pMethodInfo ) JVMX_PURE;

  virtual boost::intrusive_ptr<ObjectReference> FindJavaLangClass( const JavaString &className ) const JVMX_PURE;
  virtual boost::intrusive_ptr<ObjectReference> CreateJavaLangClassFromClassName( const boost::intrusive_ptr<JavaString> &pClassName ) JVMX_PURE;
  virtual boost::intrusive_ptr<ObjectReference> CreateJavaLangClassFromClassName( const JavaString &pClassName ) JVMX_PURE;

  virtual void ExecuteMethod( const JavaString &className, const JavaString &methodName, const JavaString &methodType, std::shared_ptr<MethodInfo> pInitialMethod ) JVMX_PURE;

  virtual size_t CalculateNumberOfStackItemsToClear( size_t exceptionRegionStart ) const JVMX_PURE;

  virtual void DoGarbageCollection() JVMX_PURE;

  virtual void StartShutdown( int exitCode ) JVMX_PURE;
  virtual void Halt( int exitCode ) JVMX_PURE;

  virtual bool IsShuttingDown() const JVMX_PURE;

  virtual int GetExitCode() const JVMX_PURE;

  virtual void RunAllFinalizers() JVMX_PURE;

  //virtual std::shared_ptr<VirtualMachine> GetVM() JVMX_PURE;

  virtual void SetExceptionThrown( boost::intrusive_ptr<ObjectReference> pException ) JVMX_PURE;
  virtual bool HasExceptionOccurred() const JVMX_PURE;
  virtual boost::intrusive_ptr<ObjectReference> GetException() JVMX_PURE;
  virtual void ResetException() JVMX_PURE;

  virtual void PushMonitor( std::shared_ptr<Lockable> pMutex ) JVMX_PURE;
  virtual std::shared_ptr<Lockable> PopMonitor() JVMX_PURE;

  virtual void PushAndZeroCallStackDepth() JVMX_PURE;
  virtual uint16_t PopCallStackDepth() JVMX_PURE;
  virtual uint16_t GetCallStackDepth() JVMX_PURE;
  virtual uint16_t IncrementCallStackDepth() JVMX_PURE;
  virtual uint16_t DecrementCallStackDepth() JVMX_PURE;

  virtual void SetJavaNativeInterface( std::weak_ptr<JavaNativeInterface> pJNI ) JVMX_PURE;

  virtual ThreadInfo ReturnCurrentThreadInfo() JVMX_PURE;
  virtual void RegisterNativeMethods( std::shared_ptr<JavaNativeInterface> pJNI ) JVMX_PURE;
  virtual void AddThread( std::shared_ptr<boost::thread> pNewThread, boost::intrusive_ptr<ObjectReference> pObject, const std::shared_ptr<IVirtualMachineState> &pNewState ) JVMX_PURE;

  virtual boost::intrusive_ptr<ObjectReference> GetCallStackOfClassObjects() JVMX_PURE;
  virtual boost::intrusive_ptr<ObjectReference> GetCallStackOfStackTraceElements() JVMX_PURE;

  virtual boost::intrusive_ptr<ObjectReference> GetClassLoaderForClassObject( boost::intrusive_ptr<ObjectReference> pObject ) JVMX_PURE;

  virtual std::vector<boost::intrusive_ptr<IJavaVariableType>> GetGCRoots() const JVMX_PURE;
  virtual std::vector<boost::intrusive_ptr<IJavaVariableType>> GetStaticObjectsAndArrays() const JVMX_PURE;

  virtual void Pause() JVMX_PURE;
  virtual void Resume() JVMX_PURE;
  virtual bool IsPaused() const JVMX_PURE;
  virtual bool IsPausing() const JVMX_PURE;

  //virtual bool GetAndClearInterruptedFlag() JVMX_PURE;
  virtual bool GetInterruptedFlag() JVMX_PURE;
  virtual void Interrupt() JVMX_PURE;

  virtual std::atomic_int64_t &GetStackLevel() JVMX_PURE;

  // Do not call. Used by the Execution Engine.
  virtual void ConfirmPaused() JVMX_PURE;

  virtual void AddGlobalReference( boost::intrusive_ptr<ObjectReference> pObject ) JVMX_PURE;
  virtual void DeleteGlobalReference( boost::intrusive_ptr<ObjectReference> pObject ) JVMX_PURE;

  virtual void AddLocalReferenceFrame() JVMX_PURE;
  virtual void DeleteLocalReferenceFrame() JVMX_PURE;

  virtual void AddLocalReference( boost::intrusive_ptr<ObjectReference> pObject ) JVMX_PURE;
  virtual void DeleteLocalReference( boost::intrusive_ptr<ObjectReference> pObject ) JVMX_PURE;

  virtual void SetExecutingNative() JVMX_PURE;
  virtual void SetExecutingHosted() JVMX_PURE;
  virtual bool IsExecutingNative() const JVMX_PURE;

  virtual void SetUserCodeStarted() JVMX_PURE;
  virtual bool HasUserCodeStarted() const JVMX_PURE;

#ifdef _DEBUG
  virtual size_t GetOperandStackSize() JVMX_PURE;
#endif // _DEBUG
};

#endif // _IVIRTUALMACHINESTATE__H_
