@echo off
echo Deleteing not needed  VISUAL 'C' files (obj lst pch bsc etc..)
be ask "Continue ? Y/N [N] "yn DEFAULT=n TIMEOUT=100
if errorlevel 2 goto no
if errorlevel 1 goto yes
goto endd
:yes
for %%x in (*.lst) do del %%x
for %%x in (*.obj) do del %%x
for %%x in (*.bsc) do del %%x
for %%x in (*.sbr) do del %%x
for %%x in (*.map) do del %%x
for %%x in (*.pch) do del %%x
for %%x in (*.pdb) do del %%x
for %%x in (*.res) do del %%x
if not exist windebug\nul goto nodeb
cd windebug
for %%x in (*.lst) do del %%x
for %%x in (*.obj) do del %%x
for %%x in (*.bsc) do del %%x
for %%x in (*.sbr) do del %%x
for %%x in (*.map) do del %%x
for %%x in (*.pch) do del %%x
for %%x in (*.pdb) do del %%x
for %%x in (*.res) do del %%x
cd ..
:nodeb
if not exist winrel\nul goto nof
cd winrel
for %%x in (*.lst) do del %%x
for %%x in (*.obj) do del %%x
for %%x in (*.bsc) do del %%x
for %%x in (*.sbr) do del %%x
for %%x in (*.map) do del %%x
for %%x in (*.pch) do del %%x
for %%x in (*.pdb) do del %%x
for %%x in (*.res) do del %%x
cd ..
:nof
echo Delete successful.
goto endd
:no
echo Delete aborted. Nothing done.
:endd
