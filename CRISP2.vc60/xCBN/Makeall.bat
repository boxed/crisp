@echo off
echo.
echo Cleaning out old libs:
echo.
if exist winrel\cbndll.lib del winrel\cbndll.lib
if exist winrel\cbndll.dll del winrel\cbndll.dll
if exist ..\cbn32\winrel\cbndll.dll   del ..\cbn32\winrel\cbndll.dll
if exist ..\cbn32\winrel\cbndll.lib   del ..\cbn32\winrel\cbndll.lib
if exist ..\cbn32\cbndll.dll          del ..\cbn32\cbndll.dll
if exist ..\cbn32\cbndll.lib          del ..\cbn32\cbndll.lib
echo.
echo Making CBNDLL DLL and testing it:
echo.
nmake /f cbndll.mak  "CFG=Win32 Release"
if errorlevel 1 goto err1
echo copying ...
copy winrel\cbndll.dll   ..\cbn32
copy winrel\cbndll.lib   ..\cbn32
copy winrel\cbndll.dll   ..\cbn32\winrel
copy winrel\cbndll.lib   ..\cbn32\winrel
rem copy cbn_.h              ..\cbn32
cd ..\cbn32
if exist  winrel\cbn32.exe del winrel\cbn32.exe
if exist  winrel\cbn32.exe del winrel\cbn32.exp
if exist  winrel\cbn32.lib del winrel\cbn32.lib
echo.
echo Making cbn32 and testing it:
echo.
call makeall
if errorlevel 1 goto err1
call exedemo
echo.
echo Coming back ...
echo.
cd ..\cbn32dll
goto end1
:err1
echo Error on make.
:end1
rem