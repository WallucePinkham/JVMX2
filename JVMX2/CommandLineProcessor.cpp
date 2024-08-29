
#include "OsFunctions.h"
#include "CommandLineProcessor.h"


inline void CommandLineProcessor::Usage(std::ostream& stream)
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
    stream << "\t\tA " << OsFunctions::GetInstance().GetPathSeparator() << " separated list of directories to search for class files.\n";
    stream << "  -D<name>=<value>";
    stream << "\t\t Set a system property";
    stream << "  -h, --help\t\tPrint this message\n";
    stream << "  -v, --version\t\tPrints version information\n";
}

inline void CommandLineProcessor::PrintVersion()
{
  std::cout << "JVMX 2 - Version " << pVersion << std::endl;
}

inline std::vector<std::string> CommandLineProcessor::Split(const std::string& input, char delimiter)
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

int CommandLineProcessor::ProcessCommandLine(int argc, char* argv[], CommandLineArguments& cmdLine)
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

      cmdLine.classPath = argv[i + 1];
      ++i;
      continue;
    }

    if (arg.length() >= 2 && (arg.substr(0, 2) == "-D"))
    {
      std::string val = arg.substr(2);
      if (val.empty())
      {
        continue;
      }

      auto arr = Split(val, '=');
      if (arr.empty() || arr[0].empty())
      {
        continue;
      }

      cmdLine.properties.push_back(Property(arr[0], arr.size() > 1 ? arr[1] : ""));
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
