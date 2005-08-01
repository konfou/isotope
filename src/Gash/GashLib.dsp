# Microsoft Developer Studio Project File - Name="GashLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=GashLib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GashLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GashLib.mak" CFG="GashLib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GashLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "GashLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GashLib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\Obj\GashLib\Release"
# PROP Intermediate_Dir "..\..\Obj\GashLib\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "QT_DLL" /D "_MT" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\GashLib.lib"

!ELSEIF  "$(CFG)" == "GashLib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\Obj\GashLib\Debug"
# PROP Intermediate_Dir "..\..\Obj\GashLib\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "QT_DLL" /D "_MT" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\GashLib_DebugVersion.lib"

!ENDIF 

# Begin Target

# Name "GashLib - Win32 Release"
# Name "GashLib - Win32 Debug"
# Begin Group "Gash"

# PROP Default_Filter "*.gash;*.proj"
# Begin Source File

SOURCE=.\MainLib\Array.gash
# End Source File
# Begin Source File

SOURCE=.\MainLib\BigInt.gash
# End Source File
# Begin Source File

SOURCE=.\MainLib\Bool.gash
# End Source File
# Begin Source File

SOURCE=.\MainLib\Console.gash
# End Source File
# Begin Source File

SOURCE=.\MainLib\Engine.gash
# End Source File
# Begin Source File

SOURCE=.\MainLib\Exception.gash
# End Source File
# Begin Source File

SOURCE=.\MainLib\Float.gash
# End Source File
# Begin Source File

SOURCE=.\MainLib\Image.gash
# End Source File
# Begin Source File

SOURCE=.\MainLib\IStream.gash
# End Source File
# Begin Source File

SOURCE=.\MainLib\Library.gash
# End Source File
# Begin Source File

SOURCE=.\MainLib\MainLib.proj
# End Source File
# Begin Source File

SOURCE=.\MainLib\make.gash
# End Source File
# Begin Source File

SOURCE=.\MainLib\PopUps.gash
# End Source File
# Begin Source File

SOURCE=.\MainLib\Project.gash
# End Source File
# Begin Source File

SOURCE=.\MainLib\Sdl.gash
# End Source File
# Begin Source File

SOURCE=.\MainLib\Stream.gash
# End Source File
# Begin Source File

SOURCE=.\MainLib\String.gash
# End Source File
# End Group
# Begin Group "MachineObjects"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\MachineObjects\MachineObjects.cpp
# End Source File
# Begin Source File

SOURCE=.\MachineObjects\MachineObjects.h
# End Source File
# Begin Source File

SOURCE=.\MachineObjects\MArray.cpp
# End Source File
# Begin Source File

SOURCE=.\MachineObjects\MArray.h
# End Source File
# Begin Source File

SOURCE=.\MachineObjects\MBigInt.cpp
# End Source File
# Begin Source File

SOURCE=.\MachineObjects\MBigInt.h
# End Source File
# Begin Source File

SOURCE=.\MachineObjects\MConsole.cpp
# End Source File
# Begin Source File

SOURCE=.\MachineObjects\MConsole.h
# End Source File
# Begin Source File

SOURCE=.\MachineObjects\MEngine.cpp
# End Source File
# Begin Source File

SOURCE=.\MachineObjects\MEngine.h
# End Source File
# Begin Source File

SOURCE=.\MachineObjects\MImage.cpp
# End Source File
# Begin Source File

SOURCE=.\MachineObjects\MImage.h
# End Source File
# Begin Source File

SOURCE=.\MachineObjects\MLibrary.cpp
# End Source File
# Begin Source File

SOURCE=.\MachineObjects\MLibrary.h
# End Source File
# End Group
# Begin Group "Test"

# PROP Default_Filter ""
# Begin Group "Engine"

# PROP Default_Filter "*.gash"
# Begin Source File

SOURCE=.\Test\Engine\arraytest.gash
# End Source File
# Begin Source File

SOURCE=.\Test\Engine\basictestneg.gash
# End Source File
# Begin Source File

SOURCE=.\Test\Engine\basictestpos.gash
# End Source File
# Begin Source File

SOURCE=.\Test\Engine\floattestpos.gash
# End Source File
# Begin Source File

SOURCE=.\Test\Engine\interfacetest.gash
# End Source File
# Begin Source File

SOURCE=.\Test\Engine\looptest.gash
# End Source File
# Begin Source File

SOURCE=.\Test\Engine\stream.gash
# End Source File
# Begin Source File

SOURCE=.\Test\Engine\stringtest.gash
# End Source File
# Begin Source File

SOURCE=.\Test\Engine\throwtest.gash
# End Source File
# End Group
# Begin Group "Class"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\Test\Class\TestAvlTree.cpp
# End Source File
# Begin Source File

SOURCE=.\Test\Class\TestAvlTree.h
# End Source File
# Begin Source File

SOURCE=.\Test\Class\TestBigNumber.cpp
# End Source File
# Begin Source File

SOURCE=.\Test\Class\TestBigNumber.h
# End Source File
# Begin Source File

SOURCE=.\Test\Class\TestDataBase.cpp
# End Source File
# Begin Source File

SOURCE=.\Test\Class\TestDataBase.h
# End Source File
# Begin Source File

SOURCE=.\Test\Class\TestHashTable.cpp
# End Source File
# Begin Source File

SOURCE=.\Test\Class\TestHashTable.h
# End Source File
# Begin Source File

SOURCE=.\Test\Class\TestMatrix.cpp
# End Source File
# Begin Source File

SOURCE=.\Test\Class\TestMatrix.h
# End Source File
# Begin Source File

SOURCE=.\Test\Class\TestPrecalcTrigTable.cpp
# End Source File
# Begin Source File

SOURCE=.\Test\Class\TestPrecalcTrigTable.h
# End Source File
# Begin Source File

SOURCE=.\Test\Class\TestSpinLock.cpp
# End Source File
# Begin Source File

SOURCE=.\Test\Class\TestSpinLock.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Test\ClassTests.cpp
# End Source File
# Begin Source File

SOURCE=.\Test\ClassTests.h
# End Source File
# Begin Source File

SOURCE=.\Test\EngineTests.cpp
# End Source File
# Begin Source File

SOURCE=.\Test\EngineTests.h
# End Source File
# Begin Source File

SOURCE=.\Test\Test.h
# End Source File
# End Group
# End Target
# End Project
