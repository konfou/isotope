# Microsoft Developer Studio Project File - Name="GashEngine" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=GashEngine - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GashEngine.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GashEngine.mak" CFG="GashEngine - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GashEngine - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "GashEngine - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GashEngine - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\Obj\GashEngine\Release"
# PROP Intermediate_Dir "..\..\Obj\GashEngine\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "_MT" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\GashEngine.lib"

!ELSEIF  "$(CFG)" == "GashEngine - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\Obj\GashEngine\Debug"
# PROP Intermediate_Dir "..\..\Obj\GashEngine\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "_MT" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"..\..\lib\GashEngine_DebugVersion.lib"

!ENDIF 

# Begin Target

# Name "GashEngine - Win32 Release"
# Name "GashEngine - Win32 Debug"
# Begin Group "Engine Source"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\engine\ClassicSyntax.cpp
# End Source File
# Begin Source File

SOURCE=.\ENGINE\Commands.cpp
# End Source File
# Begin Source File

SOURCE=.\engine\Disassembler.cpp
# End Source File
# Begin Source File

SOURCE=.\ENGINE\EClass.cpp
# End Source File
# Begin Source File

SOURCE=.\ENGINE\EInstrArray.cpp
# End Source File
# Begin Source File

SOURCE=.\ENGINE\EInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\ENGINE\EMethod.cpp
# End Source File
# Begin Source File

SOURCE=.\engine\Engine.cpp
# End Source File
# Begin Source File

SOURCE=.\engine\Error.cpp
# End Source File
# Begin Source File

SOURCE=.\ENGINE\EType.cpp
# End Source File
# Begin Source File

SOURCE=.\Engine\EvalExprResult.cpp
# End Source File
# Begin Source File

SOURCE=.\Engine\GCompiler.cpp
# End Source File
# Begin Source File

SOURCE=.\Engine\GVM.cpp
# End Source File
# Begin Source File

SOURCE=.\Engine\GVMStack.cpp
# End Source File
# Begin Source File

SOURCE=.\ENGINE\InstrSet.cpp
# End Source File
# Begin Source File

SOURCE=.\ENGINE\InstrTable.cpp
# End Source File
# Begin Source File

SOURCE=.\ENGINE\Library.cpp
# End Source File
# Begin Source File

SOURCE=.\ENGINE\Optimizer.cpp
# End Source File
# Begin Source File

SOURCE=.\ENGINE\TagNames.cpp
# End Source File
# Begin Source File

SOURCE=.\ENGINE\VarHolder.cpp
# End Source File
# End Group
# Begin Group "Engine Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\engine\ClassicSyntax.h
# End Source File
# Begin Source File

SOURCE=.\engine\Disassembler.h
# End Source File
# Begin Source File

SOURCE=.\ENGINE\EClass.h
# End Source File
# Begin Source File

SOURCE=.\ENGINE\EImplementation.h
# End Source File
# Begin Source File

SOURCE=.\ENGINE\EInstrArray.h
# End Source File
# Begin Source File

SOURCE=.\ENGINE\EInterface.h
# End Source File
# Begin Source File

SOURCE=.\ENGINE\EMethod.h
# End Source File
# Begin Source File

SOURCE=.\engine\Error.h
# End Source File
# Begin Source File

SOURCE=.\engine\ErrorMessages.h
# End Source File
# Begin Source File

SOURCE=.\ENGINE\EType.h
# End Source File
# Begin Source File

SOURCE=.\Engine\EvalExprResult.h
# End Source File
# Begin Source File

SOURCE=.\Engine\GCompiler.h
# End Source File
# Begin Source File

SOURCE=.\Engine\GVMStack.h
# End Source File
# Begin Source File

SOURCE=.\ENGINE\InstrSet.h
# End Source File
# Begin Source File

SOURCE=.\ENGINE\InstrTable.h
# End Source File
# Begin Source File

SOURCE=.\ENGINE\Optimizer.h
# End Source File
# Begin Source File

SOURCE=.\engine\TagNames.h
# End Source File
# End Group
# Begin Group "CodeObjects Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\codeobjects\Block.cpp
# End Source File
# Begin Source File

SOURCE=.\CodeObjects\Call.cpp
# End Source File
# Begin Source File

SOURCE=.\CodeObjects\Class.cpp
# End Source File
# Begin Source File

SOURCE=.\CodeObjects\CodeObject.cpp
# End Source File
# Begin Source File

SOURCE=.\codeobjects\Constant.cpp
# End Source File
# Begin Source File

SOURCE=.\codeobjects\Expression.cpp
# End Source File
# Begin Source File

SOURCE=.\CodeObjects\File.cpp
# End Source File
# Begin Source File

SOURCE=.\codeobjects\FileSet.cpp
# End Source File
# Begin Source File

SOURCE=.\codeobjects\InstrArray.cpp
# End Source File
# Begin Source File

SOURCE=.\CodeObjects\Instruction.cpp
# End Source File
# Begin Source File

SOURCE=.\CodeObjects\Interface.cpp
# End Source File
# Begin Source File

SOURCE=.\CodeObjects\Method.cpp
# End Source File
# Begin Source File

SOURCE=.\codeobjects\Operator.cpp
# End Source File
# Begin Source File

SOURCE=.\codeobjects\Project.cpp
# End Source File
# Begin Source File

SOURCE=.\codeobjects\Scope.cpp
# End Source File
# Begin Source File

SOURCE=.\codeobjects\Type.cpp
# End Source File
# Begin Source File

SOURCE=.\codeobjects\Variable.cpp
# End Source File
# Begin Source File

SOURCE=.\codeobjects\VarRef.cpp
# End Source File
# End Group
# Begin Group "CodeObjects Header"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\codeobjects\Block.h
# End Source File
# Begin Source File

SOURCE=.\CodeObjects\Call.h
# End Source File
# Begin Source File

SOURCE=.\CodeObjects\Class.h
# End Source File
# Begin Source File

SOURCE=.\CodeObjects\CodeObject.h
# End Source File
# Begin Source File

SOURCE=.\codeobjects\Constant.h
# End Source File
# Begin Source File

SOURCE=.\codeobjects\Expression.h
# End Source File
# Begin Source File

SOURCE=.\CodeObjects\File.h
# End Source File
# Begin Source File

SOURCE=.\codeobjects\FileSet.h
# End Source File
# Begin Source File

SOURCE=.\codeobjects\InstrArray.h
# End Source File
# Begin Source File

SOURCE=.\CodeObjects\Instruction.h
# End Source File
# Begin Source File

SOURCE=.\CodeObjects\Interface.h
# End Source File
# Begin Source File

SOURCE=.\CodeObjects\Method.h
# End Source File
# Begin Source File

SOURCE=.\codeobjects\Operator.h
# End Source File
# Begin Source File

SOURCE=.\codeobjects\Project.h
# End Source File
# Begin Source File

SOURCE=.\codeobjects\Scope.h
# End Source File
# Begin Source File

SOURCE=.\codeobjects\Type.h
# End Source File
# Begin Source File

SOURCE=.\codeobjects\Variable.h
# End Source File
# Begin Source File

SOURCE=.\codeobjects\VarRef.h
# End Source File
# End Group
# Begin Group "BuiltIns"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\BuiltIns\BuiltIns.cpp
# End Source File
# Begin Source File

SOURCE=.\BuiltIns\GashFloat.cpp
# End Source File
# Begin Source File

SOURCE=.\BuiltIns\GashFloat.h
# End Source File
# Begin Source File

SOURCE=.\BuiltIns\GashStream.cpp
# End Source File
# Begin Source File

SOURCE=.\BuiltIns\GashStream.h
# End Source File
# Begin Source File

SOURCE=.\BuiltIns\GashString.cpp
# End Source File
# Begin Source File

SOURCE=.\BuiltIns\GashString.h
# End Source File
# Begin Source File

SOURCE=.\BuiltIns\GObject.cpp
# End Source File
# Begin Source File

SOURCE=.\BuiltIns\GObject.h
# End Source File
# End Group
# End Target
# End Project
