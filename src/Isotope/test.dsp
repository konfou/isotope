# Microsoft Developer Studio Project File - Name="test" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=test - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "test.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "test.mak" CFG="test - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "test - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "test - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "test - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../obj/Isotope/Release"
# PROP Intermediate_Dir "../../obj/Isotope/Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "_MT" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\..\lib\SDL.lib ..\..\lib\sdlmain.lib ..\..\lib\libz.lib ..\..\lib\libpng.lib wsock32.lib /nologo /subsystem:console /machine:I386 /nodefaultlib:"libc.lib" /nodefaultlib:"libcd.lib" /out:"../../bin/Isotope.exe"

!ELSEIF  "$(CFG)" == "test - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../obj/Isotope/Debug"
# PROP Intermediate_Dir "../../obj/Isotope/Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "_MT" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib ..\..\lib\SDL.lib ..\..\lib\sdlmain.lib ..\..\lib\libz.lib ..\..\lib\libpng.lib wsock32.lib /nologo /subsystem:console /debug /machine:I386 /nodefaultlib:"libcd.lib" /nodefaultlib:"libc.lib" /out:"../../bin/IsotopeDebug.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "test - Win32 Release"
# Name "test - Win32 Debug"
# Begin Source File

SOURCE=.\AutoUpdate.cpp
# End Source File
# Begin Source File

SOURCE=.\AutoUpdate.h
# End Source File
# Begin Source File

SOURCE=.\Controller.cpp
# End Source File
# Begin Source File

SOURCE=.\Controller.h
# End Source File
# Begin Source File

SOURCE=.\GameEngine.cpp
# End Source File
# Begin Source File

SOURCE=.\GameEngine.h
# End Source File
# Begin Source File

SOURCE=.\MAnimation.cpp
# End Source File
# Begin Source File

SOURCE=.\MAnimation.h
# End Source File
# Begin Source File

SOURCE=.\MGameClient.cpp
# End Source File
# Begin Source File

SOURCE=.\MGameClient.h
# End Source File
# Begin Source File

SOURCE=.\MGameImage.cpp
# End Source File
# Begin Source File

SOURCE=.\MGameImage.h
# End Source File
# Begin Source File

SOURCE=.\MKeyPair.cpp
# End Source File
# Begin Source File

SOURCE=.\MKeyPair.h
# End Source File
# Begin Source File

SOURCE=.\MObject.cpp
# End Source File
# Begin Source File

SOURCE=.\MObject.h
# End Source File
# Begin Source File

SOURCE=.\Model.cpp
# End Source File
# Begin Source File

SOURCE=.\Model.h
# End Source File
# Begin Source File

SOURCE=.\MRealm.cpp
# End Source File
# Begin Source File

SOURCE=.\MRealm.h
# End Source File
# Begin Source File

SOURCE=.\MRealmServer.cpp
# End Source File
# Begin Source File

SOURCE=.\MRealmServer.h
# End Source File
# Begin Source File

SOURCE=.\MScriptEngine.cpp
# End Source File
# Begin Source File

SOURCE=.\MScriptEngine.h
# End Source File
# Begin Source File

SOURCE=.\MService.h
# End Source File
# Begin Source File

SOURCE=.\MSpot.cpp
# End Source File
# Begin Source File

SOURCE=.\MSpot.h
# End Source File
# Begin Source File

SOURCE=.\MStore.cpp
# End Source File
# Begin Source File

SOURCE=.\MStore.h
# End Source File
# Begin Source File

SOURCE=.\NRealmProtocol.cpp
# End Source File
# Begin Source File

SOURCE=.\NRealmProtocol.h
# End Source File
# Begin Source File

SOURCE=.\VEntropyCollector.cpp
# End Source File
# Begin Source File

SOURCE=.\VEntropyCollector.h
# End Source File
# Begin Source File

SOURCE=.\VGame.cpp
# End Source File
# Begin Source File

SOURCE=.\VGame.h
# End Source File
# Begin Source File

SOURCE=.\View.cpp
# End Source File
# Begin Source File

SOURCE=.\View.h
# End Source File
# Begin Source File

SOURCE=.\ViewPort.cpp
# End Source File
# Begin Source File

SOURCE=.\ViewPort.h
# End Source File
# Begin Source File

SOURCE=.\VMainMenu.cpp
# End Source File
# Begin Source File

SOURCE=.\VMainMenu.h
# End Source File
# Begin Source File

SOURCE=.\VServer.cpp
# End Source File
# Begin Source File

SOURCE=.\VServer.h
# End Source File
# Begin Source File

SOURCE=.\VWave.cpp
# End Source File
# Begin Source File

SOURCE=.\VWave.h
# End Source File
# End Target
# End Project
