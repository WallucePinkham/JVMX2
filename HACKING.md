# Hacking JVMX2

## Compiling

I have used Microsoft Visual Studio 2022 to compile the code in x86 Debug and Release Mode.
You need:

* [vcpkg](https://github.com/microsoft/vcpkg)
* [wallaroo](https://wallaroolib.sourceforge.net/index.html) v0.7.0

### Boost

Boost dependencies are declared in `vcpkg.json` and pinned to v1.78.0 through its vcpkg baseline. The manifest includes the Boost modules used directly by JVMX2; Boost.Thread and Boost.ProgramOptions provide compiled libraries, and their transitive dependencies are included by vcpkg. The project overlay triplet selects the installed MSVC v142 toolset for Boost 1.78, whose build scripts do not correctly match newer MSVC toolset library names.

Install vcpkg and enable its MSBuild integration once:

```powershell
vcpkg integrate install
```

Open the solution in Visual Studio or build it with MSBuild. The project enables manifest mode and vcpkg will install the pinned dependencies for the selected platform. To install them manually from the repository root, use the x86 overlay triplet: `vcpkg install --triplet x86-windows --overlay-triplets .\vcpkg-triplets`. The overlay expects the MSVC v142 toolset to be installed; it has been tested with the v142 compiler bundled alongside Visual Studio 2022.

Wallaroo does not need compiling. Point Visual Studio to its include folder under **C/C++ > Additional Include Directories**.

Remember to use the Win32 build. JVMX does not run on x64 yet, mostly because I still need to do the work for JNI on x64 architectures.

## Running

To run JVMX2, you need GNU Classpath v0.99.0. Compiling GNU Classpath on Windows is not easy without WSL, so I recommend using that.

You can compile it using:
`./configure --disable-gtk-peer --disable-gconf-peer --disable-jni  --build=x86_64-linux-gnu`

You will need to have antlr-2.7.5.jar and Java 7 installed. I just copied the antlr jar file into my classpath folder, but there are better ways.

You can copy the output of the build to the `<root folder>/JVMX2/JVMX2/classpath` folder since that is where JVMX2 will search for it.

This should allow you to run, debug and hack JVMX2.
