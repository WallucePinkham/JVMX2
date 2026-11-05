@echo off
call compile.bat
call "C:\Program Files\Java\jdk1.7.0_80\bin\jar" cfe .\AllTests.jar AllTestsRunner *.class