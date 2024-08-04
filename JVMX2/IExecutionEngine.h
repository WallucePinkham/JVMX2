#ifndef _IEXECUTIONENGINE__H_
#define _IEXECUTIONENGINE__H_

#include <memory>
#include <wallaroo/part.h>

#include "boost/intrusive_ptr.hpp"

#include "GlobalConstants.h"
//#include "IncreaseCallStackDepth.h"
//#include "JavaExceptionConstants.h"

class JavaNativeInterface;
class IVirtualMachineState; // Forward Declaration
class ILogger; // Forward Declaration
class IClassLibrary;
class MethodInfo;
class JavaObject;

class IExecutionEngine JVMX_ABSTRACT : public wallaroo::Part
{
public:
  virtual ~IExecutionEngine() JVMX_NOEXCEPT {};

  virtual void Run( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState ) JVMX_PURE;

  virtual void ThrowJavaException( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, const JVMX_ANSI_CHAR_TYPE *javaExceptionName ) JVMX_PURE;
  virtual void ThrowJavaException( IVirtualMachineState *pVirtualMachineState, const JVMX_ANSI_CHAR_TYPE *javaExceptionName ) JVMX_PURE;

  virtual std::shared_ptr<MethodInfo> IdentifyVirtualMethodToCall( const std::shared_ptr<IVirtualMachineState> & pVirtualMachineState, std::shared_ptr<MethodInfo> pMethodInfo, boost::intrusive_ptr<ObjectReference> pObject ) JVMX_PURE;

  virtual void Halt() JVMX_PURE;

protected:
  IExecutionEngine() {};
};

#endif // _IEXECUTIONENGINE__H_
