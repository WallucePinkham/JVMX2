# JVMX2

An implementation of a Java 7 Virtual Machine in C++.

## CRITICAL: Execution Environment (Windows PowerShell Only)

The execution environment is **pure Windows PowerShell (`powershell.exe`)**. 

⚠️ **Executing `grep`, `cat`, `head`, `tail`, `ls`, or `sed` causes a fatal `CommandNotFoundException` and halts the workflow.**

Whenever you want to search or inspect files, you **MUST** map to these exact commands:

| Instead of... | You MUST use... |
| :--- | :--- |
| `grep -r "text" .` | `Select-String -Pattern "text" -Path .\* -Include *.cpp,*.h` |
| `grep "text" file.cpp` | `Select-String -Pattern "text" -Path .\file.cpp` |
| `cat file.cpp` | `Get-Content .\file.cpp` |
| `head -n 20 file.cpp` | `Get-Content .\file.cpp -TotalCount 20` |
| `tail -n 20 file.cpp` | `Get-Content .\file.cpp -Tail 20` |
| `find . -name "*.h"` | `Get-ChildItem -Recurse -Filter *.h` |

NEVER USE PRECOMPILED HEADERS, as it will cause compilation to fail.

## Environment & Toolchain

- **OS**: Windows (PowerShell environment only).
- **IDE / Compiler**: Microsoft Visual Studio 2022 (MSVC).
- **Language Standard**: **STRICTLY C++11**. Newer features (C++14, C++17, C++20) are forbidden.
- **Key External Dependencies**: Boost, Wallaroo, Zlib / Zip.
- **Specification**: Refer to the official Java Virtual Machine Specification for **Java SE 7**.

---

## Building

When you need to build the soltion use the following parameters for compiling a debug build:
/JMC /permissive- /ifcOutput "Debug\" /GS /analyze- /W3 /Zc:wchar_t /I"..\wallaroo" /I"..\boost_1_78_0" /ZI /Gm- /Od /sdl /Fd"Debug\vc143.pdb" /Zc:inline /fp:precise /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "JVMX_LOG_VERBOSE" /D "_UNICODE" /D "UNICODE" /errorReport:prompt /WX- /Zc:forScope /RTC1 /Gd /Oy- /MDd /FC /Fa"Debug\" /EHsc /nologo /Fo"Debug\" /Fp"Debug\JVMX2.pch" /diagnostics:column 

Use the following parameters for the linker:
/OUT:"C:\dev\JVMX2\Debug\JVMX2.exe" /MANIFEST /PROFILE /NXCOMPAT /PDB:"C:\dev\JVMX2\Debug\JVMX2.pdb" /DYNAMICBASE "ws2_32.lib" "kernel32.lib" "user32.lib" "gdi32.lib" "winspool.lib" "comdlg32.lib" "advapi32.lib" "shell32.lib" "ole32.lib" "oleaut32.lib" "uuid.lib" "odbc32.lib" "odbccp32.lib" /DEBUG /MACHINE:X86 /INCREMENTAL /PGD:"C:\dev\JVMX2\Debug\JVMX2.pgd" /SUBSYSTEM:CONSOLE /MANIFESTUAC:"level='asInvoker' uiAccess='false'" /ManifestFile:"Debug\JVMX2.exe.intermediate.manifest" /LTCGOUT:"Debug\JVMX2.iobj" /ERRORREPORT:PROMPT /ILK:"Debug\JVMX2.ilk" /NOLOGO /TLBID:1 

## Command Line & Workflow (PowerShell Only)

Do **NOT** use Linux / bash utilities (`grep`, `head`, `tail`, `sed`, `cat`). Use PowerShell equivalents:
- Search text: `Select-String -Pattern "text" -Path .\*.cpp`
- View file content: `Get-Content .\file.cpp -Head 20`
- Find files: `Get-ChildItem -Recurse -Filter *.h`

### Core Commands
- **Git Branch Check**: Always verify the current branch before proposing or making changes:
  ```powershell
  git branch --show-current
  ```
- **Build**:
  ```powershell
  $vswhere = "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
  $installationPath = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -property installationPath
  $msbuild = Join-Path $installationPath "MSBuild\Current\Bin\MSBuild.exe"
  & $msbuild .\JVMX2.sln /t:Build /m /p:Configuration=Debug /p:Platform=x86
  ```
  The `x86` solution platform maps to the project's `Win32` platform. Build outputs are written to `Debug`.
- **Run Tests**: Execute from directory `C:/dev/JVMX2/JVMX2`:
  ```powershell
  ..\Debug\JVMX2.exe -cp ..\JVMX2 -jar ..\tests\alltests.jar
  ```

  Running tests in Debug mode is VERY SLOW because of a lot of extra logging.
  If you want to run tests, consider compiling in Release mode and running:

  ```powershell
  ..\Release\JVMX2.exe -cp ..\JVMX2 -jar ..\tests\alltests.jar
  ```

  OR Compile without JVMX_LOG_VERBOSE being defined. However, debugging can be hard if you do this.



---

## Coding Standards & Style

Study `BasicStackManager.h` and `BasicStackManager.cpp` as the reference standard for style and structure.
- **DO NOT** adopt coding styles from third-party libraries (`boost`, `wallaroo`, or the `Zip` directory).
- **Precompiled Headers**: **Forbidden**. Do NOT use or generate PCH files (`stdafx.h` / `pch.h`).

### C++11 Guardrails (CRITICAL)
Gemini must strictly avoid modern C++ idioms that break on C++11:
- **NO `std::make_unique`** (Introduced in C++14). Initialize `std::unique_ptr` using explicit `new`:
  ```cpp
  // Correct in C++11:
  std::unique_ptr<MyClass> ptr(new MyClass(arg));
  ```
- **NO `std::string_view`, `std::optional`, `std::variant`, or `std::any`** (C++17).
- **NO structured bindings** (`auto [x, y] = pair;` is C++17).
- **NO generic lambdas** or return type deduction `auto func() { return x; }` (C++14).
- **Constexpr**: Stick to C++11 simple single-return `constexpr`.

### Memory, Headers & Design Rules
- **RAII First**: Use `std::unique_ptr`, `std::shared_ptr`, or stack allocation. **Never** use raw `delete` or manage manual lifetimes.
- **Header Guards**: Every header file must include **both** `#pragma once` AND traditional include guards:
  ```cpp
  #pragma once
  #ifndef __MY_HEADER_H__
  #define __MY_HEADER_H__

  // ... declarations ...

  #endif // __MY_HEADER_H__
  ```
- **Cast Rules**: Absolutely **no C-style casts** `(Type)val`. Use `static_cast`, `reinterpret_cast`, or `dynamic_cast`.
- **Namespaces**: **Never** write `using namespace std;` (especially in headers).
- **Macros**: Avoid `#define` for logic or constants. Use `constexpr`, `const`, or inline functions.
- **Immutability**: Make classes immutable wherever possible (`const` members and methods).
- **Thread Safety**: Preserve existing thread-safety contracts and locking mechanisms (e.g., mutexes, locks).

---

## Restrictions & Safety

- **Allowed**: Reading local source files, inspecting Java 7 specifications, running tests, and executing MSBuild/linters.
- **Disallowed**: 
  - Do NOT fix any compilation errors or warnings in files you did not change. Only fix errors in files you changed.
- Do NOT install or introduce new third-party dependencies.
  - Do NOT upgrade existing libraries (`boost`, `wallaroo`, etc.) without explicit user permission.
  - Do NOT modify CI/CD pipelines or project build properties without consent.
  - Do NOT push to Git remotes.

---

## Step-by-Step Workflow

1. Check current branch (`git branch --show-current`).
2. Make the smallest necessary working change.
3. Build the solution and run tests via PowerShell.
4. **Never claim tests have passed without running the command and verifying the exit output.**

---

## Related Documentation
Before making significant architectural changes, review:
- `README.md`
- `Design.md`
- `Hacking.md`
```

