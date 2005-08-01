# Microsoft Developer Studio Project File - Name="GashQt" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=GashQt - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GashQt.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GashQt.mak" CFG="GashQt - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GashQt - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "GashQt - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GashQt - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\Obj\GashQt\Release"
# PROP Intermediate_Dir "..\..\Obj\GashQt\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "$(QTDIR)\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "QT_DLL" /D "QT_THREAD_SUPPORT" /D "_MT" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\GashQt.lib"

!ELSEIF  "$(CFG)" == "GashQt - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "GashQt___Win32_Debug"
# PROP BASE Intermediate_Dir "GashQt___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\Obj\GashQt\Debug"
# PROP Intermediate_Dir "..\..\Obj\GashQt\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "$(QTDIR)\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "QT_DLL" /D "QT_THREAD_SUPPORT" /D "_MT" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\GashQt_DebugVersion.lib"

!ENDIF 

# Begin Target

# Name "GashQt - Win32 Release"
# Name "GashQt - Win32 Debug"
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\IDE\DebugDialogBase.ui

!IF  "$(CFG)" == "GashQt - Win32 Release"

!ELSEIF  "$(CFG)" == "GashQt - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\IDE
InputPath=.\IDE\DebugDialogBase.ui
InputName=DebugDialogBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\IDE\FileViewDialogBase.ui

!IF  "$(CFG)" == "GashQt - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\IDE
InputPath=.\IDE\FileViewDialogBase.ui
InputName=FileViewDialogBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "GashQt - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\IDE
InputPath=.\IDE\FileViewDialogBase.ui
InputName=FileViewDialogBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ide\GetStringDialogBase.ui

!IF  "$(CFG)" == "GashQt - Win32 Release"

!ELSEIF  "$(CFG)" == "GashQt - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\ide
InputPath=.\ide\GetStringDialogBase.ui
InputName=GetStringDialogBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\IDE\importcpp.ui

!IF  "$(CFG)" == "GashQt - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\IDE
InputPath=.\IDE\importcpp.ui
InputName=importcpp

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "GashQt - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\IDE
InputPath=.\IDE\importcpp.ui
InputName=importcpp

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\IDE\SaveFormatDialogBase.ui

!IF  "$(CFG)" == "GashQt - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\IDE
InputPath=.\IDE\SaveFormatDialogBase.ui
InputName=SaveFormatDialogBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "GashQt - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\IDE
InputPath=.\IDE\SaveFormatDialogBase.ui
InputName=SaveFormatDialogBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\IDE\TestDialogBase.ui

!IF  "$(CFG)" == "GashQt - Win32 Release"

!ELSEIF  "$(CFG)" == "GashQt - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\IDE
InputPath=.\IDE\TestDialogBase.ui
InputName=TestDialogBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\Ide\UsageDialogBase.ui

!IF  "$(CFG)" == "GashQt - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\Ide
InputPath=.\Ide\UsageDialogBase.ui
InputName=UsageDialogBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ELSEIF  "$(CFG)" == "GashQt - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\Ide
InputPath=.\Ide\UsageDialogBase.ui
InputName=UsageDialogBase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\IDE\wassaildialogbase.ui

!IF  "$(CFG)" == "GashQt - Win32 Release"

!ELSEIF  "$(CFG)" == "GashQt - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Uic'ing $(InputName).ui ...
InputDir=.\IDE
InputPath=.\IDE\wassaildialogbase.ui
InputName=wassaildialogbase

BuildCmds= \
	%qtdir%\bin\uic.exe $(InputPath) -o $(InputDir)\$(InputName).h \
	%qtdir%\bin\uic.exe $(InputPath) -i $(InputName).h -o $(InputDir)\$(InputName).cpp \
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp \
	

"$(InputDir)\$(InputName).h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
   $(BuildCmds)
# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Generated Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\IDE\DebugDialogBase.cpp
# End Source File
# Begin Source File

SOURCE=.\ide\GetStringDialogBase.cpp
# End Source File
# Begin Source File

SOURCE=.\IDE\importcpp.cpp
# End Source File
# Begin Source File

SOURCE=.\IDE\moc_DebugDialogBase.cpp
# End Source File
# Begin Source File

SOURCE=.\IDE\moc_FileViewDialogBase.cpp
# End Source File
# Begin Source File

SOURCE=.\ide\moc_GetStringDialogBase.cpp
# End Source File
# Begin Source File

SOURCE=.\IDE\moc_importcpp.cpp
# End Source File
# Begin Source File

SOURCE=.\IDE\moc_importcppimpl.cpp
# End Source File
# Begin Source File

SOURCE=.\IDE\moc_SaveFormatDialogBase.cpp
# End Source File
# Begin Source File

SOURCE=.\IDE\moc_TestDialogBase.cpp
# End Source File
# Begin Source File

SOURCE=.\IDE\moc_UsageDialogBase.cpp
# End Source File
# Begin Source File

SOURCE=.\IDE\moc_WassailDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\IDE\moc_wassaildialogbase.cpp
# End Source File
# Begin Source File

SOURCE=.\IDE\SaveFormatDialogBase.cpp
# End Source File
# Begin Source File

SOURCE=.\IDE\TestDialogBase.cpp
# End Source File
# Begin Source File

SOURCE=.\IDE\UsageDialogBase.cpp
# End Source File
# Begin Source File

SOURCE=.\IDE\wassaildialogbase.cpp
# End Source File
# End Group
# Begin Group "Generated Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\IDE\DebugDialogBase.h
# End Source File
# Begin Source File

SOURCE=.\ide\GetStringDialogBase.h
# End Source File
# Begin Source File

SOURCE=.\IDE\importcpp.h
# End Source File
# Begin Source File

SOURCE=.\IDE\SaveFormatDialogBase.h
# End Source File
# Begin Source File

SOURCE=.\IDE\TestDialogBase.h
# End Source File
# Begin Source File

SOURCE=.\IDE\UsageDialogBase.h
# End Source File
# Begin Source File

SOURCE=.\IDE\wassaildialogbase.h
# End Source File
# End Group
# Begin Group "IDE Source"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat;for;f90"
# Begin Source File

SOURCE=.\IDE\DebugBreakPointManager.cpp
# End Source File
# Begin Source File

SOURCE=.\IDE\DebugDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\IDE\DebuggerListViewItem.cpp
# End Source File
# Begin Source File

SOURCE=.\IDE\DebuggerVariableItem.cpp
# End Source File
# Begin Source File

SOURCE=.\IDE\DebugSourceManager.cpp
# End Source File
# Begin Source File

SOURCE=.\IDE\DebugStackManager.cpp
# End Source File
# Begin Source File

SOURCE=.\IDE\FileViewDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\IDE\FileViewDialogBase.cpp
# End Source File
# Begin Source File

SOURCE=.\ide\GetStringDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\IDE\HardCodedImages.cpp
# End Source File
# Begin Source File

SOURCE=.\IDE\importcppimpl.cpp
# End Source File
# Begin Source File

SOURCE=.\IDE\SaveFormatDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\ide\SplashScreen.cpp
# End Source File
# Begin Source File

SOURCE=.\IDE\TestDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\IDE\UsageDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\IDE\wassaildialog.cpp
# End Source File
# End Group
# Begin Group "IDE Header"

# PROP Default_Filter "h;hpp;hxx;hm;inl;fi;fd"
# Begin Source File

SOURCE=.\IDE\DebugBreakPointManager.h
# End Source File
# Begin Source File

SOURCE=.\IDE\DebugDialog.h
# End Source File
# Begin Source File

SOURCE=.\IDE\DebuggerListViewItem.h
# End Source File
# Begin Source File

SOURCE=.\IDE\DebuggerVariableItem.h
# End Source File
# Begin Source File

SOURCE=.\IDE\DebugSourceManager.h
# End Source File
# Begin Source File

SOURCE=.\IDE\DebugStackManager.h
# End Source File
# Begin Source File

SOURCE=.\IDE\FileViewDialog.h
# End Source File
# Begin Source File

SOURCE=.\IDE\FileViewDialogBase.h
# End Source File
# Begin Source File

SOURCE=.\ide\GetStringDialog.h
# End Source File
# Begin Source File

SOURCE=.\IDE\HardCodedImages.h
# End Source File
# Begin Source File

SOURCE=.\IDE\importcppimpl.h

!IF  "$(CFG)" == "GashQt - Win32 Release"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\IDE
InputPath=.\IDE\importcppimpl.h
InputName=importcppimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ELSEIF  "$(CFG)" == "GashQt - Win32 Debug"

# PROP Ignore_Default_Tool 1
# Begin Custom Build - Moc'ing $(InputName).h ...
InputDir=.\IDE
InputPath=.\IDE\importcppimpl.h
InputName=importcppimpl

"$(InputDir)\moc_$(InputName).cpp" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	%qtdir%\bin\moc.exe $(InputDir)\$(InputName).h -o $(InputDir)\moc_$(InputName).cpp

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\IDE\SaveFormatDialog.h
# End Source File
# Begin Source File

SOURCE=.\ide\SplashScreen.h
# End Source File
# Begin Source File

SOURCE=.\IDE\TestDialog.h
# End Source File
# Begin Source File

SOURCE=.\IDE\UsageDialog.h
# End Source File
# Begin Source File

SOURCE=.\IDE\wassaildialog.h
# End Source File
# End Group
# Begin Group "Qt MachineObjects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\QtCallBacks\Popups.cpp
# End Source File
# Begin Source File

SOURCE=.\QtCallBacks\Popups.h
# End Source File
# Begin Source File

SOURCE=.\QtCallBacks\QtMachineObjects.cpp
# End Source File
# End Group
# End Target
# End Project
