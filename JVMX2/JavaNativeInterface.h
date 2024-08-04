#ifndef _JAVANATIVEINTERFACE__H_
#define _JAVANATIVEINTERFACE__H_

#include <memory>
#include <vector>
#include <list>

#include <wallaroo/part.h>
#include <wallaroo/collaborator.h>

#include "GlobalConstants.h"
#include "JavaTypes.h"

#include "IVirtualMachineState.h"
#include "ILogger.h"

#include "jni_internal.h"
#include "NativeLibraryContainer.h"

class JavaNativeInterfaceLibrary;

enum class e_IsFunctionStatic : uint8_t
{
  No = 0,
  Yes,
};

class JavaNativeInterface : public wallaroo::Part
{
public:
  JavaNativeInterface();

  virtual ~JavaNativeInterface();

  void SetVMState( std::shared_ptr<IVirtualMachineState> pVMState );

  static boost::intrusive_ptr<JavaString> MakeJNINameWithoutArgumentDescriptor( boost::intrusive_ptr<JavaString> pClassName, boost::intrusive_ptr<JavaString> pMethodName );

  void RegisterFunction( const JavaString &name, void *pFunction );

  void ExecuteFunctionStatic( boost::intrusive_ptr<JavaString> pName, boost::intrusive_ptr<JavaString> pParameterTypes, boost::intrusive_ptr<ObjectReference> pJavaLangClass );
  void ExecuteFunction( boost::intrusive_ptr<JavaString> pName, boost::intrusive_ptr<JavaString> pParameterTypes, boost::intrusive_ptr<ObjectReference> pObject );

  bool LoadOsLibrary( const JVMX_ANSI_CHAR_TYPE *pLibraryName );

  JNIEnv *GetEnvironment();
  JavaVM *GetVM();

private:
  typedef std::pair< bool, std::shared_ptr<void> > ParameterValue;
  typedef std::list< ParameterValue > ParameterStack;

private:
  //ParameterStack SetupFunctionParameters( boost::intrusive_ptr<JavaString> pParameterTypes );

  void ExecuteFunctionInternal( boost::intrusive_ptr<JavaString> pName, boost::intrusive_ptr<JavaString> pParameterTypes, boost::intrusive_ptr<ObjectReference> pObjectOrClass, e_IsFunctionStatic isStatic );

  void ExecuteFunctionStdCall( void *pFunction, int *pIntParams, int paramCount, char16_t returnType, boost::intrusive_ptr<ObjectReference> pObjectOrClass );
  static intptr_t ConvertReferencePointerToIntValue( boost::intrusive_ptr<IJavaVariableType> pValue );
  void PushParametersStandardCall( int *pInteger, std::vector<boost::intrusive_ptr<JavaString> > parameterList, size_t sizeAsIntegers );

  std::shared_ptr<ILogger> GetLogger();
  const std::shared_ptr<IVirtualMachineState> & GetVMState();

private:
  std::map< JavaString, void * > m_Functions;
  std::shared_ptr<IVirtualMachineState> m_pVMState;

  JavaVMExported *m_pExportedVM;
  //JavaVMInternal m_Vm;

  //JNIEnvInternal m_JNI;
  JNIEnvExported *m_pExportedEnvironment;
#ifdef _DEBUG
  std::thread::id m_DebugThreadID;
#endif // _DEBUG
};

#endif // _JAVANATIVEINTERFACE__H_

void ExecuteFunctionStdCall();
