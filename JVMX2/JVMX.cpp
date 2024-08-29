// JVMX.cpp : Defines the entry point for the console application.
//

#include <tchar.h>
#include <iostream>
#include <memory>

#include <boost/program_options.hpp>

#include "GlobalConstants.h"
#include "DefaultClassLoader.h"

#include "BasicVirtualMachineState.h"
#include "VirtualMachine.h"
#include "CommandLineProcessor.h"


int main(int argc, char* argv[])
{
  CommandLineArguments cmdLine;

  int commandLineResult = CommandLineProcessor::ProcessCommandLine(argc, argv, cmdLine);
  if (commandLineResult != 0)
  {
    return commandLineResult;
  }

  DefaultClassLoader loader;

  try
  {
    std::shared_ptr<VirtualMachine> pJVM = VirtualMachine::Create();
    std::shared_ptr<BasicVirtualMachineState> pInitialState = std::make_shared<BasicVirtualMachineState>(pJVM);

    pJVM->Initialise(cmdLine.mainClass.empty() ? cmdLine.jarFile : cmdLine.mainClass, cmdLine.classPath, cmdLine.properties, pInitialState);

    std::string fileName;
    if (!cmdLine.mainClass.empty())
    {
      fileName = cmdLine.mainClass;
    }
    else
    {
      fileName = cmdLine.jarFile;
    }

    //pJVM->Run(JVMX_T("..\\Tests\\InitialiseCharset.class"), pInitialState, false);

    //pJVM->Run( JVMX_T( "..\\Tests\\TestFloat.class" ), pInitialState, false );
    //pJVM->Run(JVMX_T("..\\Tests\\TestLanguageFeatures.class"), pInitialState, false);
    //pJVM->Run(JVMX_T("..\\Tests\\TestThreads.class"), pInitialState, false);

    JavaString fileNameJs = JavaString::FromCString(fileName.c_str());
    if (fileNameJs.EndsWith(u".class"))
    {
      pJVM->Run(fileNameJs, pInitialState, false);
    }
    else
    {
      pJVM->RunClassName(fileNameJs, pInitialState, false);
    }
    

    //pJVM->Run( JVMX_T( "TestThreads.class" ), pInitialState );
    //pJVM->Run( JVMX_T( "TestFontNames.class" ), pInitialState );

    //pJVM->Run( JVMX_T( "HelloWorld.class" ), pInitialState );
    //pJVM->Run( JVMX_T( "TestFrame.class" ), pInitialState );
    //pJVM->Run( JVMX_T( "TestAWTButton.class" ), pInitialState );
    //pJVM->Run( JVMX_T( "TestAWTGame.class" ), pInitialState );
    //pJVM->Run( JVMX_T( "TestAWTMenu.class" ), pInitialState );
    //pJVM->Run( JVMX_T( "TestIPC.class" ), pInitialState );
    //pJVM->Run( JVMX_T( "TestThreads2.class" ), pInitialState );
    //pJVM->Run( JVMX_T( "TestProperties.class" ), pInitialState );
    //pJVM->Run( JVMX_T( "TestNullPointerException.class" ), pInitialState );

    pJVM->Stop(pInitialState);

    /*pGarbageCollector->Collect( e_ForceGarbageCollection::Yes );*/
  }
  catch (JVMXException& ex)
  {
    std::cout << ex.what() << std::endl;
  }

  return 0;
}