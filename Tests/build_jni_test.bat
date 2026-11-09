@echo off
setlocal
pushd "%~dp0"
if errorlevel 1 exit /b 1

set "VSWHERE=C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath`) do set "VSINSTALL=%%i"
if not defined VSINSTALL goto failed

call "%VSINSTALL%\VC\Auxiliary\Build\vcvarsall.bat" x86
if errorlevel 1 goto failed

python generate_jni_stub_probes.py
if errorlevel 1 goto failed

cl /nologo /LD /EHsc /MD /I"..\JVMX2" /I"..\JVMX2\include" TestJNIEntrypoints.cpp /link /DEF:TestJNIEntrypoints.def /OUT:TestJNIEntrypoints.dll
if errorlevel 1 goto failed

del /q TestJNIStubProbes.inc
popd
exit /b 0

:failed
if exist TestJNIStubProbes.inc del /q TestJNIStubProbes.inc
popd
exit /b 1
