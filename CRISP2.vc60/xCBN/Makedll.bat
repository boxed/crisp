@echo off
if exist winrel\cbndll.lib del winrel\cbndll.lib
if exist winrel\cbndll.dll del winrel\cbndll.dll
if exist ..\cbndemo\winrel\cbndll.dll del ..\cbndemo\winrel\cbndll.dll
if exist ..\cbndemo\winrel\cbndll.lib del ..\cbndemo\winrel\cbndll.lib
if exist ..\cbndemo\cbndll.dll        del ..\cbndemo\cbndll.dll
if exist ..\cbndemo\cbndll.lib        del ..\cbndemo\cbndll.lib
nmake /f cbndll.mak  "CFG=Win32 Release"
if errorlevel 1 goto err
copy winrel\cbndll.dll ..\cbndemo
copy winrel\cbndll.lib ..\cbndemo
copy winrel\cbndll.dll ..\cbndemo\winrel
copy winrel\cbndll.lib ..\cbndemo\winrel
copy cbn_.h   ..\cbndemo
cd ..\cbndemo
winrel\cbndemo
cd ..\cbndll
goto end
:err
echo Error on make
:end
rem

