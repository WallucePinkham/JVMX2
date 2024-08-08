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

int _tmain( int argc, _TCHAR *argv[] )
{
  boost::program_options::options_description optionsDesc( "Usage" );

  optionsDesc.add_options()
  ( "help", "Produces this message." )
  ( "-jar", "Jar file to load." )
  ( "starting-classfile,f", boost::program_options::value<std::string>(), "ClassFile to run, e.g. Boom.class." );

  boost::program_options::positional_options_description positionalDesc;
  positionalDesc.add( "starting-classfile", 1 );

  boost::program_options::variables_map variablesMap;
  boost::program_options::store( boost::program_options::wcommand_line_parser( argc, argv ).options( optionsDesc ).positional( positionalDesc ).run(), variablesMap );

  boost::program_options::notify( variablesMap );

  DefaultClassLoader loader;

  try
  {
    std::shared_ptr<VirtualMachine> pJVM = VirtualMachine::Create();

    std::shared_ptr<BasicVirtualMachineState> pInitialState = std::make_shared<BasicVirtualMachineState>( pJVM );

    pJVM->Initialise( variablesMap, pInitialState );

    auto fileName = variablesMap["starting-classfile"];

    //pJVM->Run(JVMX_T("..\\Tests\\InitialiseCharset.class"), pInitialState, false);

    //pJVM->Run( JVMX_T( "..\\Tests\\TestFloat.class" ), pInitialState, false );
    //pJVM->Run(JVMX_T("..\\Tests\\TestLanguageFeatures.class"), pInitialState, false);
    //pJVM->Run(JVMX_T("..\\Tests\\TestThreads.class"), pInitialState, false);

    JavaString fileNameJs = JavaString::FromCString(fileName.as<std::string>().c_str());
    pJVM->Run(fileNameJs.ToCharacterArray(), pInitialState, false);
    
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

    pJVM->Stop( pInitialState );

    /*pGarbageCollector->Collect( e_ForceGarbageCollection::Yes );*/
  }
  catch ( JVMXException &ex )
  {
    std::cout << ex.what() << std::endl;
  }

  return 0;
}