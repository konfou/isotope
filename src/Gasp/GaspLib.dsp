# Microsoft Developer Studio Project File - Name="GaspLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=GaspLib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GaspLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GaspLib.mak" CFG="GaspLib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GaspLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "GaspLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GaspLib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\Obj\GaspLib\Release"
# PROP Intermediate_Dir "..\..\Obj\GaspLib\Release"
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
# ADD LIB32 /nologo /out:"..\..\lib\GaspLib.lib"

!ELSEIF  "$(CFG)" == "GaspLib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\Obj\GaspLib\Debug"
# PROP Intermediate_Dir "..\..\Obj\GaspLib\Debug"
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
# ADD LIB32 /nologo /out:"..\..\lib\GaspLibDbg.lib"

!ENDIF 

# Begin Target

# Name "GaspLib - Win32 Release"
# Name "GaspLib - Win32 Debug"
# Begin Group "Gasp"

# PROP Default_Filter "*.Gasp;*.proj"
# Begin Source File

SOURCE=.\MainLib\Array.gasp
# End Source File
# Begin Source File

SOURCE=.\MainLib\BigInt.gasp
# End Source File
# Begin Source File

SOURCE=.\MainLib\Bool.gasp
# End Source File
# Begin Source File

SOURCE=.\MainLib\Console.gasp
# End Source File
# Begin Source File

SOURCE=.\MainLib\Engine.gasp
# End Source File
# Begin Source File

SOURCE=.\MainLib\Exception.gasp
# End Source File
# Begin Source File

SOURCE=.\MainLib\Float.gasp
# End Source File
# Begin Source File

SOURCE=.\MainLib\Image.gasp
# End Source File
# Begin Source File

SOURCE=.\MainLib\IStream.gasp
# End Source File
# Begin Source File

SOURCE=.\MainLib\Library.gasp
# End Source File
# Begin Source File

SOURCE=.\MainLib\MainLib.proj
# End Source File
# Begin Source File

SOURCE=.\MainLib\make.gasp
# End Source File
# Begin Source File

SOURCE=.\MainLib\PopUps.gasp
# End Source File
# Begin Source File

SOURCE=.\MainLib\Project.gasp
# End Source File
# Begin Source File

SOURCE=.\MainLib\Sdl.gasp
# End Source File
# Begin Source File

SOURCE=.\MainLib\Stream.gasp
# End Source File
# Begin Source File

SOURCE=.\MainLib\String.gasp
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

# PROP Default_Filter "*.Gasp"
# Begin Source File

SOURCE=.\Test\Engine\arraytest.gasp
# End Source File
# Begin Source File

SOURCE=.\Test\Engine\basictestneg.gasp
# End Source File
# Begin Source File

SOURCE=.\Test\Engine\basictestpos.gasp
# End Source File
# Begin Source File

SOURCE=.\Test\Engine\castneg.gasp
# End Source File
# Begin Source File

SOURCE=.\Test\Engine\castpos.gasp
# End Source File
# Begin Source File

SOURCE=.\Test\Engine\floattestpos.gasp
# End Source File
# Begin Source File

SOURCE=.\Test\Engine\interfacetest.gasp
# End Source File
# Begin Source File

SOURCE=.\Test\Engine\looptest.gasp
# End Source File
# Begin Source File

SOURCE=.\Test\Engine\poundtest.gasp
# End Source File
# Begin Source File

SOURCE=.\Test\Engine\serializetest.gasp
# End Source File
# Begin Source File

SOURCE=.\Test\Engine\stream.gasp
# End Source File
# Begin Source File

SOURCE=.\Test\Engine\stringtest.gasp
# End Source File
# Begin Source File

SOURCE=.\Test\Engine\throwtest.gasp
# End Source File
# Begin Source File

SOURCE=.\Test\Engine\virtualtest.gasp
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
