# JVMX2 Cross-Platform Porting Guide

This guide tracks the work required to make JVMX2 portable while preserving its existing Windows x86 build. The immediate focus is Linux under Ubuntu on WSL; the eventual targets are Linux x86 and x86_64 and Windows x86 and x64. Keep the existing Windows build working throughout the port.

## Target matrix

| Operating system | Architecture | Status / priority |
|---|---|---|
| Windows | x86 | Existing build; preserve |
| Linux | x86_64 | Immediate porting target |
| Linux | x86 | Immediate porting target |
| Windows | x64 | Future target |

Treat each OS/architecture combination as a distinct ABI target. In particular, Windows x64 uses the Microsoft x64 calling convention, while Linux x86_64 uses the System V AMD64 ABI; one is not a substitute for the other.

## Current constraints

- The Windows build remains defined by Visual Studio project files. A separate Linux-only CMake build definition is now in place.
- `OperatingSystemLinux` implements the Linux OS delegate and is selected on Linux. Per-thread suspension and resumption remain explicitly unsupported.
- `JavaNativeInterface.cpp` retains MSVC x86 inline assembly for Windows and uses libffi with the platform ABI for Linux x86_64.
- JVMX2 uses Boost headers and compiled libraries. Dependencies are declared in `vcpkg.json`; the Linux x86_64 vcpkg installation and CMake build are complete.
- Platform-facing strings use `JVMX_T`; Java UTF-16 literals use `JVMX_UTF16_T`. Java strings are stored as UTF-16 code units and do not use Linux's 32-bit `wchar_t`.

## Porting milestones

### 1. Establish a repeatable Linux build

- A Linux-only CMake build definition is in place; keep it separate from the Visual Studio project.
- The Linux OS delegate and core portability changes pass targeted GCC 11 C++11 syntax checks, and the full Linux x86_64 CMake build succeeds.
- The Linux x86_64 CMake build completes with libffi installed from the vcpkg manifest.
- The documented `-jar` test invocation initializes system classes, then aborts with an uncaught `std::bad_cast` while loading the file URL handler.
- Running the extracted `AllTestsRunner.class` directly with a temporary Linux build of `TestJNIEntrypoints` reached the JNI entrypoint checks (226 total, 171 unimplemented-entry failures), then failed the runtime access-control test. This is useful partial coverage, not a passing full suite.
- Record the exact configure, build, and test commands.

For WSL, bootstrap vcpkg, then configure with the vcpkg toolchain. Use the repository's existing `vcpkg_installed` root so the partially completed Linux install is reused. vcpkg keeps packages for different triplets in separate subdirectories (`x64-linux`, `x86-windows`, and so on), so installing Linux dependencies does not replace Windows packages.

```sh
cmake -S . -B build/linux-x64 \
  -DCMAKE_TOOLCHAIN_FILE=/mnt/c/dev/vcpkg/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_TARGET_TRIPLET=x64-linux \
  -DVCPKG_INSTALLED_DIR="$PWD/vcpkg_installed" \
  -DVCPKG_INSTALL_OPTIONS="--x-buildtrees-root=$HOME/.cache/jvmx2/vcpkg-buildtrees;--x-packages-root=$HOME/.cache/jvmx2/vcpkg-packages"
cmake --build build/linux-x64
```

From the repository root, run the current test entry point with:

```sh
build/linux-x64/JVMX2 -cp JVMX2 -jar Tests/AllTests.jar
```

The test class files under `JVMX2/classpath` must be available to the Linux runtime.

CMake's vcpkg toolchain installs any missing manifest dependencies into the `x64-linux` tree during configuration. Keep vcpkg's temporary build and package-staging trees in WSL's Linux filesystem rather than under `/mnt/c` to reduce cross-filesystem overhead.

### 2. Complete and validate the Linux OS delegate

The `OperatingSystemLinux` implementation and OS selection/build wiring are in place. Validate the behavior on Linux and keep the Windows implementation available on Windows.

Port the interface behavior in focused groups:

- Library loading and symbol lookup: `dlopen`, `dlsym`, and `dlclose`.
- Directory checks and canonical paths: POSIX filesystem APIs and UTF-8 paths, preserving the JVMX character-conversion contracts.
- Thread naming, process ID, and host name.
- Path, file, and line separators.
- IPv4 helpers and socket-related behavior.
- Thread suspension and resumption only after defining the required semantics.

Do not treat `SIGSTOP`/`SIGCONT` as direct replacements for `SuspendThread`/`ResumeThread`: these signals can stop or continue the entire process, not just the requested thread. First trace the callers and expected behavior. If the JVM relies on suspending an individual thread, choose a safe design or report the operation as unsupported rather than introducing process-wide stops.

### 3. Replace the JNI call mechanism

The current JNI dispatcher pushes arguments and calls a function using MSVC x86 inline assembly. That is specific to 32-bit Windows and does not implement the Linux x86 ABI, the x86_64 System V ABI, or the Windows x64 ABI.

- Document the native argument and return-value types that JVMX2 must support.
- Linux x86_64 dynamic JNI calls use libffi and `FFI_DEFAULT_ABI`; Windows x86 keeps its existing dispatcher. Other ABI targets remain unsupported pending separate implementations and validation.
- `vcpkg.json` declares libffi, and the Linux CMake target links the package's `libffi` target.
- The Linux runtime recognizes the class library's legacy built-in DLL aliases as internal libraries rather than trying to load them as Linux shared objects.
- The temporary JNI test library's `runAll` call completed through the Linux dispatcher and returned its `jlong` result; built-in native calls also exercised reference, integer, boolean, and void signatures. Floating-point native signatures still need focused validation.
- Add focused native-method tests that cover mixed integer/floating-point arguments and all return types before treating the Linux dispatcher as fully validated.

Do not try to solve the JNI port by mechanically translating the existing assembly to GCC extended assembly; calling conventions and register/stack rules differ across architectures and operating systems.

### 4. Remove remaining compiler and platform assumptions

- Find active MSVC-only syntax, Windows headers/APIs, and inline-assembly breakpoints in compiled sources.
- Replace debug breakpoints with a small platform-appropriate helper or another repository-consistent mechanism.
- Keep C++11 compatibility and preserve existing thread-safety behavior.
- Exclude third-party source directories from broad mechanical edits unless a specific required fix is identified.

### 5. Validate both targets

- Build and run the applicable JVM tests on x86_64 Ubuntu.
- Build and run the same tests on x86 Ubuntu (using a 32-bit toolchain and libraries).
- Include tests for OS delegation and JNI native calls; a successful compile alone does not establish ABI correctness.
- Keep known expected runtime failures distinct from build failures and document them in test results.

### 6. Add Windows x64 support

After the Linux targets are working, add and validate the Windows x64 build without regressing Windows x86. Reuse the ABI-aware JNI design only where it explicitly supports the Microsoft x64 ABI; validate Windows x64 native calls independently rather than inferring support from Linux x86_64 tests.

## References

- OS interface: `JVMX2/IOperatingSystemDelegate.h`
- Windows implementation: `JVMX2/OperatingSystemWindows.h` and `JVMX2/OperatingSystemWindows.cpp`
- JNI dispatcher: `JVMX2/JavaNativeInterface.cpp`
- Dependency manifest: `vcpkg.json`
- Windows build and test notes: `HACKING.md`
