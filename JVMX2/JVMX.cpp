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

const char* pVersion = "v0.0.1-alpha";

void Usage(std::ostream& stream)
{
  stream << "Usage: JVMX2 [options] <mainclass> [arguments]\n";
  stream << "             (to execute a class)\n";
  stream << "    or JVMX2 [options] -jar <jarfile> [arguments]\n";
  stream << "             (to execute a jar file)\n";
  stream << "\n";
  stream << "Arguments following the the main class or jarfile are passed";
  stream << "as the arguments to the main class.\n";
  stream << "\n";
  stream << "Options:\n";
  stream << "  -cp, --class-path <class search path of directories>\n";
  stream << "\t\tA ; separated list of directories to search for class files.\n";
  stream << "  -h, --help\t\tPrint this message\n";
  stream << "  -v, --version\t\tPrints version information\n";
}

struct CommandLine
{
public:
  std::string mainClass;
  std::string jarFile;
  std::vector<std::string> classPath;
  std::vector<std::string> classArguments;
  std::string currentExe;
};

void PrintVersion()
{
  std::cout << "JVMX 2 - Version " << pVersion << std::endl;
}

std::vector<std::string> Split(const std::string& input, char delimiter)
{
  std::vector<std::string> result;
  auto copy = input;

  auto pos = copy.find(delimiter);
  while (pos != std::string::npos)
  {
    result.push_back(copy.substr(0, pos));
    copy = copy.substr(pos + 1);
    pos = copy.find(delimiter);
  }

  result.push_back(copy);
  return result;
}

int ProcessCommandLine(int argc, char* argv[], CommandLine& cmdLine)
{
  if (argc == 1)
  {
    Usage(std::cerr);
    return 1;
  }

  cmdLine.currentExe = argv[0];

  bool mainClassFound = false;

  for (int i = 1; i < argc; ++i)
  {
    std::string arg = argv[i];
    if (arg[0] != '-' && !mainClassFound)
    {
      cmdLine.mainClass = arg.c_str();
      mainClassFound = true;
      continue;
    }

    if (arg == "-jar")
    {
      if (i == argc - 1)
      {
        std::cerr << "Error: missing jar file\n\n";
        Usage(std::cerr);
        return 1;
      }

      cmdLine.jarFile = argv[i + 1];
      ++i;
      mainClassFound = true;
      continue;
    }

    if (mainClassFound)
    {
      cmdLine.classArguments.push_back(argv[i]);
      continue;
    }

    if (arg == "-h" || arg == "--help")
    {
      Usage(std::cout);
      return 0;
    }

    if (arg == "-v" || arg == "--version")
    {
      PrintVersion();
      return 0;
    }

    if (arg == "-cp" || arg == "--class-path")
    {
      if (i + 1 >= argc)
      {
        std::cerr << "Error: missing classpath\n\n";
        Usage(std::cerr);
        return 1;
      }

      std::string classPathString = argv[i + 1];
      cmdLine.classPath = Split(classPathString, ';');
      ++i;
      continue;
    }

    Usage(std::cerr);
    return 1;
  }

  if (!mainClassFound)
  {
    std::cerr << "Error: missing class name or jar\n\n";
    Usage(std::cerr);
    return 1;
  }

  return 0;
}

int main(int argc, char* argv[])
{
  CommandLine cmdLine;

  int commandLineResult = ProcessCommandLine(argc, argv, cmdLine);
  if (commandLineResult != 0)
  {
    return commandLineResult;
  }

  DefaultClassLoader loader;

  try
  {
    std::shared_ptr<VirtualMachine> pJVM = VirtualMachine::Create();
    std::shared_ptr<BasicVirtualMachineState> pInitialState = std::make_shared<BasicVirtualMachineState>(pJVM);

    pJVM->Initialise(cmdLine.mainClass.empty() ? cmdLine.jarFile : cmdLine.mainClass, pInitialState);

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

    pJVM->Stop(pInitialState);

    /*pGarbageCollector->Collect( e_ForceGarbageCollection::Yes );*/
  }
  catch (JVMXException& ex)
  {
    std::cout << ex.what() << std::endl;
  }

  return 0;
}