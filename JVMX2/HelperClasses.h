#ifndef _HELPERCLASSES__H_
#define _HELPERCLASSES__H_

#include <memory>
#include "IVirtualMachineState.h"
#include "JavaClass.h"

class HelperClasses
{
public:
  static bool DoesClassImplementInterface(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, 
                                          std::shared_ptr<JavaClass> pClass, 
                                          boost::intrusive_ptr<JavaString> nameOfInterface);

  static bool DoesClassImplementInterface(IVirtualMachineState *pVirtualMachineState,
                                          std::shared_ptr<JavaClass> pClass,
                                          const JavaString * const nameOfInterface);

  static bool IsSuperClassOf(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, 
                             boost::intrusive_ptr<JavaString> pPossibleSuperClassName, 
                             boost::intrusive_ptr<JavaString> pDerivedClassName);

  static bool IsSuperClassOf(IVirtualMachineState *pVirtualMachineState, 
                             const JavaString* const pPossibleSuperClassName,
                             const JavaString* const pDerivedClassName);


  static std::shared_ptr<JavaClass> ResolveClass(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, 
                                                 const JavaString& className);

  static std::shared_ptr<JavaClass> ResolveClass(IVirtualMachineState *pVirtualMachineState,
                                                 const JavaString& className);

  static void ThrowJavaException(const std::shared_ptr<IVirtualMachineState>& pVirtualMachineState, const JVMX_ANSI_CHAR_TYPE* javaExceptionName);
  static void ThrowJavaException(IVirtualMachineState* pVirtualMachineState, const JVMX_ANSI_CHAR_TYPE* javaExceptionName);

  static void ThrowJavaExceptionInternal(IVirtualMachineState* pVirtualMachineState, boost::intrusive_ptr<ObjectReference> pException);

};

#endif // !_HELPERCLASSES__H_

