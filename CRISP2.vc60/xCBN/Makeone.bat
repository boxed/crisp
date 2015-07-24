@echo off
echo.
echo Cleaning out old libs:
echo.
if exist winrel\cbndll.lib del winrel\cbndll.lib
if exist winrel\cbndll.dll del winrel\cbndll.dll
echo.
echo Making CBNDLL DLL:
echo.
nmake /f cbndll.mak  "CFG=Win32 Release"
if errorlevel 1 goto err
echo.
echo updateing to demo directory:
echo.
update winrel\cbndll.dll   ..\cbn32
update winrel\cbndll.dll   ..\cbn32\winrel
update winrel\cbndll.lib   ..\cbn32
update winrel\cbndll.lib   ..\cbn32\winrel
update cbn_.h              ..\cbn32

echo.
echo Updating to VB demo directory:
echo.
update winrel\cbndll.dll   ..\vbtest
update winrel\cbndll.lib   ..\vbtest
update cbn_.h              ..\vbtest
goto end1
:err
echo Error on make
:end1