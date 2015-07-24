@echo off
echo.
echo Cleaning out old libs:
echo.
if exist winrel\cbndll.lib del winrel\cbndll.lib
if exist winrel\cbndll.dll del winrel\cbndll.dll
echo.
echo Making CBNDLL DLL:
echo.
nmake /f cbndll.mak  "CFG=Win32 Debug"
if errorlevel 1 goto err
echo.
echo updateing to demo directory:
echo.

echo.
echo Updating to VB demo directory:
echo.
goto end1
:err
echo Error on make
:end1