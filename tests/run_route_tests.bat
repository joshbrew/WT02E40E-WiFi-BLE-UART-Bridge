@echo off
setlocal
cd /d "%~dp0.."
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul
if errorlevel 1 exit /b 1
if not exist build\route-tests mkdir build\route-tests
cl /nologo /W4 /WX /std:c11 /Isrc tests\test_tx_route.c src\wt_tx_route.c /Febuild\route-tests\test_tx_route.exe /Fobuild\route-tests\
if errorlevel 1 exit /b 1
build\route-tests\test_tx_route.exe
exit /b %errorlevel%
