# Microsoft Visual C++ Generated NMAKE File, Format Version 2.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=Win32 Debug
!MESSAGE No configuration specified.  Defaulting to Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "Win32 Release" && "$(CFG)" != "Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "CBNDLL.MAK" CFG="Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

################################################################################
# Begin Project
# PROP Target_Last_Scanned "Win32 Debug"
MTL=MkTypLib.exe
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "WinRel"
# PROP BASE Intermediate_Dir "WinRel"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "WinRel"
# PROP Intermediate_Dir "WinRel"
OUTDIR=.\WinRel
INTDIR=.\WinRel

ALL : $(OUTDIR)/CBNDLL.dll $(OUTDIR)/CBNDLL.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE CPP /nologo /MT /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /MT /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /FR /c
CPP_PROJ=/nologo /MT /W3 /GX /YX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /FR$(INTDIR)/ /Fp$(OUTDIR)/"CBNDLL.pch" /Fo$(INTDIR)/ /c 
CPP_OBJS=.\WinRel/
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"CBN.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"CBNDLL.bsc" 
BSC32_SBRS= \
	$(INTDIR)/CBN_EXT.SBR \
	$(INTDIR)/CBN_MAIN.SBR \
	$(INTDIR)/DUMP.SBR \
	$(INTDIR)/CBN_95.SBR \
	$(INTDIR)/CBN_NT.SBR \
	$(INTDIR)/MARXCODE.SBR \
	$(INTDIR)/CBN_ASM.SBR

$(OUTDIR)/CBNDLL.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:windows /DLL /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:windows /DLL /MACHINE:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /NOLOGO /SUBSYSTEM:windows /DLL /INCREMENTAL:no\
 /PDB:$(OUTDIR)/"CBNDLL.pdb" /MACHINE:I386 /DEF:".\CBNDLL.DEF"\
 /OUT:$(OUTDIR)/"CBNDLL.dll" /IMPLIB:$(OUTDIR)/"CBNDLL.lib" 
DEF_FILE=.\CBNDLL.DEF
LINK32_OBJS= \
	$(INTDIR)/CBN_EXT.OBJ \
	$(INTDIR)/CBN_MAIN.OBJ \
	$(INTDIR)/CBN_95.OBJ \
	$(INTDIR)/CBN_NT.OBJ \
	$(INTDIR)/MARXCODE.OBJ \
	$(INTDIR)/CBN_ASM.OBJ \
	$(INTDIR)/CBN.res

#	$(INTDIR)/DUMP.OBJ \

$(OUTDIR)/CBNDLL.dll : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS) $(INTDIR)/dump.obj
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "WinDebug"
# PROP BASE Intermediate_Dir "WinDebug"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "WinDebug"
# PROP Intermediate_Dir "WinDebug"
OUTDIR=.\WinDebug
INTDIR=.\WinDebug

ALL : $(OUTDIR)/CBNDLL.dll $(OUTDIR)/CBNDLL.bsc

$(OUTDIR) : 
    if not exist $(OUTDIR)/nul mkdir $(OUTDIR)

# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE CPP /nologo /MT /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /c
# ADD CPP /nologo /MT /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /FR /c
CPP_PROJ=/nologo /MT /W3 /GX /Zi /YX /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /FR$(INTDIR)/ /Fp$(OUTDIR)/"CBNDLL.pch" /Fo$(INTDIR)/ /Fd$(OUTDIR)/"CBNDLL.pdb"\
 /c 
CPP_OBJS=.\WinDebug/
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo$(INTDIR)/"CBN.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o$(OUTDIR)/"CBNDLL.bsc" 
BSC32_SBRS= \
	$(INTDIR)/CBN_EXT.SBR \
	$(INTDIR)/CBN_MAIN.SBR \
	$(INTDIR)/DUMP.SBR \
	$(INTDIR)/CBN_95.SBR \
	$(INTDIR)/CBN_NT.SBR \
	$(INTDIR)/MARXCODE.SBR \
	$(INTDIR)/CBN_ASM.SBR

$(OUTDIR)/CBNDLL.bsc : $(OUTDIR)  $(BSC32_SBRS)
    $(BSC32) @<<
  $(BSC32_FLAGS) $(BSC32_SBRS)
<<

LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:windows /DLL /DEBUG /MACHINE:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NOLOGO /SUBSYSTEM:windows /DLL /DEBUG /MACHINE:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /NOLOGO /SUBSYSTEM:windows /DLL /INCREMENTAL:yes\
 /PDB:$(OUTDIR)/"CBNDLL.pdb" /DEBUG /MACHINE:I386 /DEF:".\CBNDLL.DEF"\
 /OUT:$(OUTDIR)/"CBNDLL.dll" /IMPLIB:$(OUTDIR)/"CBNDLL.lib" 
DEF_FILE=.\CBNDLL.DEF
LINK32_OBJS= \
	$(INTDIR)/CBN_EXT.OBJ \
	$(INTDIR)/CBN_MAIN.OBJ \
	$(INTDIR)/DUMP.OBJ \
	$(INTDIR)/CBN_95.OBJ \
	$(INTDIR)/CBN_NT.OBJ \
	$(INTDIR)/MARXCODE.OBJ \
	$(INTDIR)/CBN_ASM.OBJ \
	$(INTDIR)/CBN.res

$(OUTDIR)/CBNDLL.dll : $(OUTDIR)  $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Group "Source Files"

################################################################################
# Begin Source File

SOURCE=.\CBN_EXT.C
DEP_CBN_E=\
	.\MARXDEV.H\
	.\INTERFCE.H\
	.\CBN_.H\
	.\MARXCODE.C\
	.\DUMP.H\
	.\CBN_PROG.H

$(INTDIR)/CBN_EXT.OBJ :  $(SOURCE)  $(DEP_CBN_E) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CBN_MAIN.C
DEP_CBN_M=\
	.\MYDEFINE.H\
	.\CBN_.H\
	.\INTERFCE.H\
	.\CBN_SER.C\
	.\CBN_IDS.C\
	.\CBN_RAMS.C\
	.\CBN_CRYP.C\
	.\CBN_IDEA.C\
	.\CODEIDEA.C\
	.\CBN_PROG.H\
	.\MACROS.H\
	.\IDEA.H\
	.\IDEA.C

$(INTDIR)/CBN_MAIN.OBJ :  $(SOURCE)  $(DEP_CBN_M) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CBNDLL.DEF
# End Source File
################################################################################
# Begin Source File

SOURCE=.\DUMP.C
DEP_DUMP_=\
	.\DUMP.H

$(INTDIR)/DUMP.OBJ :  $(SOURCE)  $(DEP_DUMP_) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CBN_95.C

$(INTDIR)/CBN_95.OBJ :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CBN_NT.C

$(INTDIR)/CBN_NT.OBJ :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\MARXCODE.C

$(INTDIR)/MARXCODE.OBJ :  $(SOURCE)  $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CBN_ASM.C
DEP_CBN_A=\
	.\MARXDEV.H\
	.\_CBN_ASM.H\
	.\CBN_ASM.H\
	.\CBNDEL.C\
	.\CBNRES.C\
	.\CBNRDY.C\
	.\CBNTEST.C\
	.\CBNSTU.C

$(INTDIR)/CBN_ASM.OBJ :  $(SOURCE)  $(DEP_CBN_A) $(INTDIR)

# End Source File
################################################################################
# Begin Source File

SOURCE=.\CBN.RC

$(INTDIR)/CBN.res :  $(SOURCE)  $(INTDIR)
   $(RSC) $(RSC_PROJ)  $(SOURCE) 

# End Source File
# End Group
# End Project
################################################################################
