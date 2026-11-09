@echo off
pushd "%~dp0"
if errorlevel 1 exit /b 1

call build_jni_test.bat
if errorlevel 1 goto failed
python generate_access_test.py
if errorlevel 1 goto failed
python generate_wide_branch_tests.py
if errorlevel 1 goto failed
call "C:\Program Files\Java\jdk1.7.0_80\bin\javac.exe" *.java
if errorlevel 1 goto failed

popd
exit /b 0

:failed
popd
exit /b 1