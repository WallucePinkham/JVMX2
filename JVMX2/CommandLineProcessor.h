#ifndef _COMMANDDLINE_PROCESSOR__H_
#define _COMMANDDLINE_PROCESSOR__H_

#include <string>
#include <vector>
#include <iostream>

static const char* pVersion = "v0.0.1-alpha";

struct CommandLineArguments
{
public:
  std::string mainClass;
  std::string jarFile;
  std::vector<std::string> classPath;
  std::vector<std::string> classArguments;
  std::string currentExe;
};

class CommandLineProcessor
{
public:
  static int ProcessCommandLine(int argc, char* argv[], CommandLineArguments& cmdLine);

private:
  static void Usage(std::ostream& stream);
  static void PrintVersion();
  static std::vector<std::string> Split(const std::string& input, char delimiter);
};

#endif // _COMMANDDLINE_PROCESSOR__H_
