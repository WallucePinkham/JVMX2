# Hacking JVMX2

## Compiling

I have used Microsoft Visual Studio 2022 to compile the code in x86 Debug and Release Mode.
You need to download:

* [Boost](https://www.boost.org/) v1.67.0
* [wallaroo](https://wallaroolib.sourceforge.net/index.html) v0.7.0

Unzip and extract these somewhere on your hard drive. 

### Compile Boost

Next compile the Boost libraries. On my system, the boost libraries all compiled to library files like: `libboost_atomic-vc140-mt-sgd-x64-1_67.lib`
But when trying to compile JVMX2, the linker was looking for: `libboost_atomic-vc141-mt-sgd-x64-1_67.lib`. Renaming / copying the files from vc140 to vc141 seemed to work. I suspect a more modern version of Boost will solve this problem, but I have not experimented yet, because I am not sure what else will break.

### Wallaroo

Wallaroo does not need compiling.

### Point Visual Studio in the right direction

Now you just have to point Visual Studio in the direction of the boost include folder, the boost binaries and the wallaroo include folder.

* Set your `Library Directories` under `VC++ Directories` to e.g. `<root folder>\boost_1_67_0\stage\lib;$(LibraryPath)`. 
* Set your `Additional Include Directories` under `C/C++` to e.g. `<root folder>\boost_1_67_0;<root folder>\wallaroo;%(AdditionalIncludeDirectories)` 
* Set your `Additional Dependencies` under `Linker Input` (under `Linker`) to include `ws2_32.lib`

Remeber to use Win32 build. JVMX does not run on x64 just yet, mostly because I am not super familiar with x64 assembly. I do intend to get there, but if you don't want to wait, pull requests are welcome.

## Running

To run JVMX2, you need GNU Classpath v0.99.0. Compiling GNU Classpath on Windows is not easy without WSL, so I recommend using that.

You can compile it using:
`./configure --disable-gtk-peer --disable-gconf-peer --disable-jni  --build=x86_64-linux-gnu`

You will need to have antlr-2.7.5.jar and Java 7 installed. I just copied the antlr jar file into my classpath folder, but there are better ways.

You can copy the output of the build to the `<root folder>/JVMX2/JVMX2/classpath` folder since that is where JVMX2 will search for it.

This should allow you to run, debug and hack JVMX2.
