@echo off
pushd "%~dp0"
if errorlevel 1 exit /b 1
call compile.bat
if errorlevel 1 goto failed
call "C:\Program Files\Java\jdk1.7.0_80\bin\jar.exe" cfe .\AllTests.jar AllTestsRunner *.class
if errorlevel 1 goto failed

popd
exit /b 0

:failed
popd
exit /b 1
