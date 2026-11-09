# JVMX2

The application is an implementation of a Java Virtual Machine in C++.

## Tech Stack

- **Compiler**: Microsoft Visual Studio 2022
- **Key Libraries**: boost, wallaroo, zib
- **Tests**: execute `..\Release\JVMX2.exe -cp ..\JVMX2 -jar ..\tests\alltests.jar` from `C:/dev/JVMX2/JVMX2` (Release mode preferred for test performance)

Use powershell commands and NOT bash to execute commands, because you are on Windows. You do NOT have access to `grep`, `head`, `sed` and the like use windows equivalents or Powershell commands instead.

Always verify the current git branch using `git branch --show-current` before making changes or suggesting commits.

## Coding Standards

Look at files like `BasicStackManager.h` and `BasicStackManager.cpp` for good examples. DO NOT follow standards from the boost or wallaroo libraries. DO Not follow coding standards from code in the `Zip` folder.

### Must DO

- **C++11 Standard stritness**: STRICTLY stick to C++ 11 features. Do NOT use any newer C++ language features.
- **RAII First**: Use `std::unique_ptr`, `std::shared_ptr`, or stack allocation. **Never** use raw `new`/`delete`.
- **Headers**: Use `#pragma once` AND include guards. Always use both. Include guard names should start and end with `__`
- **Immutable classes**: Wherever possible, make classes immutable.
- **Keep thread safety**: Don't change thread safety models (e.g. removing mutexes)

### Must Avoid

- **No C-Style Casts**: Use `static_cast`, `dynamic_cast`, etc. C-style casts `(int)x` are forbidden.
- **No Macros**: Avoid `#define` for logic. Use `constexpr` or `inline` functions.
- **No `using namespace std;`**: ever.
- **No precompiled headers**: DO NOT use pre-compiled headers

## Restrictions

Allowed: Reading source files, running linters. Running tests. You are allowed to refernce the specifications for the Java 7 VM online or locally.

Disallowed: Installing new dependencies, modifying CI/CD configs, database schema changes, or pushing to git.
DO NOT upgrade the dependent libraries, without human consent.

## Workflow

1. Make the smallest working change.
2. Never claim tests passed unless you ran them.

## See also

Refer to `Design.md` and `Hacking.md` as well as `README.md`.
