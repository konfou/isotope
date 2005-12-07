# Microsoft Developer Studio Project File - Name="GClasses" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=GClasses - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "GClasses.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "GClasses.mak" CFG="GClasses - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "GClasses - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "GClasses - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "GClasses - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\Obj\GClasses\Release"
# PROP Intermediate_Dir "..\..\Obj\GClasses\Release"
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
# ADD LIB32 /nologo /out:"..\..\lib\GClasses.lib"

!ELSEIF  "$(CFG)" == "GClasses - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\Obj\GClasses\Debug"
# PROP Intermediate_Dir "..\..\Obj\GClasses\Debug"
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
# ADD LIB32 /nologo /out:"..\..\lib\GClasses_DebugVersion.lib"

!ENDIF 

# Begin Target

# Name "GClasses - Win32 Release"
# Name "GClasses - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\GArff.cpp
# End Source File
# Begin Source File

SOURCE=.\GArff.h
# End Source File
# Begin Source File

SOURCE=.\GArray.cpp
# End Source File
# Begin Source File

SOURCE=.\GArray.h
# End Source File
# Begin Source File

SOURCE=.\GAVLTree.cpp
# End Source File
# Begin Source File

SOURCE=.\GAVLTree.h
# End Source File
# Begin Source File

SOURCE=.\GBezier.cpp
# End Source File
# Begin Source File

SOURCE=.\GBezier.h
# End Source File
# Begin Source File

SOURCE=.\GBigNumber.cpp
# End Source File
# Begin Source File

SOURCE=.\GBigNumber.h
# End Source File
# Begin Source File

SOURCE=.\GBillboardCamera.cpp
# End Source File
# Begin Source File

SOURCE=.\GBillboardCamera.h
# End Source File
# Begin Source File

SOURCE=.\GBitTable.cpp
# End Source File
# Begin Source File

SOURCE=.\GBitTable.h
# End Source File
# Begin Source File

SOURCE=.\GBuffer.h
# End Source File
# Begin Source File

SOURCE=.\GCompress.cpp
# End Source File
# Begin Source File

SOURCE=.\GCompress.h
# End Source File
# Begin Source File

SOURCE=.\GCppParser.cpp
# End Source File
# Begin Source File

SOURCE=.\GCppParser.h
# End Source File
# Begin Source File

SOURCE=.\GCrypto.cpp
# End Source File
# Begin Source File

SOURCE=.\GCrypto.h
# End Source File
# Begin Source File

SOURCE=.\GDaftLearner.cpp
# End Source File
# Begin Source File

SOURCE=.\GDaftLearner.h
# End Source File
# Begin Source File

SOURCE=.\GDataBase.cpp
# End Source File
# Begin Source File

SOURCE=.\GDataBase.h
# End Source File
# Begin Source File

SOURCE=.\GDate.cpp
# End Source File
# Begin Source File

SOURCE=.\GDate.h
# End Source File
# Begin Source File

SOURCE=.\GDecisionTree.cpp
# End Source File
# Begin Source File

SOURCE=.\GDecisionTree.h
# End Source File
# Begin Source File

SOURCE=.\GDiff.cpp
# End Source File
# Begin Source File

SOURCE=.\GDiff.h
# End Source File
# Begin Source File

SOURCE=.\GDirectSound.h
# End Source File
# Begin Source File

SOURCE=.\GDirList.cpp
# End Source File
# Begin Source File

SOURCE=.\GDirList.h
# End Source File
# Begin Source File

SOURCE=.\GDSound.h
# End Source File
# Begin Source File

SOURCE=.\GEZSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\GEZSocket.h
# End Source File
# Begin Source File

SOURCE=.\GFile.cpp
# End Source File
# Begin Source File

SOURCE=.\GFile.h
# End Source File
# Begin Source File

SOURCE=.\GFindFile.h
# End Source File
# Begin Source File

SOURCE=.\GFlipTable.h
# End Source File
# Begin Source File

SOURCE=.\GFourier.cpp
# End Source File
# Begin Source File

SOURCE=.\GFourier.h
# End Source File
# Begin Source File

SOURCE=.\GGenetic.cpp
# End Source File
# Begin Source File

SOURCE=.\GGenetic.h
# End Source File
# Begin Source File

SOURCE=.\GGrammar.cpp
# End Source File
# Begin Source File

SOURCE=.\GGrammar.h
# End Source File
# Begin Source File

SOURCE=.\GHardFont.cpp
# End Source File
# Begin Source File

SOURCE=.\GHardFont.h
# End Source File
# Begin Source File

SOURCE=.\GHashTable.cpp
# End Source File
# Begin Source File

SOURCE=.\GHashTable.h
# End Source File
# Begin Source File

SOURCE=.\GHeap.cpp
# End Source File
# Begin Source File

SOURCE=.\GHeap.h
# End Source File
# Begin Source File

SOURCE=.\GHtml.cpp
# End Source File
# Begin Source File

SOURCE=.\GHtml.h
# End Source File
# Begin Source File

SOURCE=.\GHttp.cpp
# End Source File
# Begin Source File

SOURCE=.\GHttp.h
# End Source File
# Begin Source File

SOURCE=.\GImage.cpp
# End Source File
# Begin Source File

SOURCE=.\GImage.h
# End Source File
# Begin Source File

SOURCE=.\GKeyPair.cpp
# End Source File
# Begin Source File

SOURCE=.\GKeyPair.h
# End Source File
# Begin Source File

SOURCE=.\GKNN.cpp
# End Source File
# Begin Source File

SOURCE=.\GKNN.h
# End Source File
# Begin Source File

SOURCE=.\GLList.cpp
# End Source File
# Begin Source File

SOURCE=.\GLList.h
# End Source File
# Begin Source File

SOURCE=.\GMacros.h
# End Source File
# Begin Source File

SOURCE=.\GManifold.cpp
# End Source File
# Begin Source File

SOURCE=.\GManifold.h
# End Source File
# Begin Source File

SOURCE=.\GMath.cpp
# End Source File
# Begin Source File

SOURCE=.\GMath.h
# End Source File
# Begin Source File

SOURCE=.\GMatrix.cpp
# End Source File
# Begin Source File

SOURCE=.\GMatrix.h
# End Source File
# Begin Source File

SOURCE=.\GMemChunk.h
# End Source File
# Begin Source File

SOURCE=.\GMipsEmulator.cpp
# End Source File
# Begin Source File

SOURCE=.\GMipsEmulator.h
# End Source File
# Begin Source File

SOURCE=.\GNaiveBayes.cpp
# End Source File
# Begin Source File

SOURCE=.\GNaiveBayes.h
# End Source File
# Begin Source File

SOURCE=.\GNeuralNet.cpp
# End Source File
# Begin Source File

SOURCE=.\GNeuralNet.h
# End Source File
# Begin Source File

SOURCE=.\GParseTree.cpp
# End Source File
# Begin Source File

SOURCE=.\GParseTree.h
# End Source File
# Begin Source File

SOURCE=.\GPipe.cpp
# End Source File
# Begin Source File

SOURCE=.\GPipe.h
# End Source File
# Begin Source File

SOURCE=.\GPNG.cpp
# End Source File
# Begin Source File

SOURCE=.\GPNG.h
# End Source File
# Begin Source File

SOURCE=.\GPointerQueue.cpp
# End Source File
# Begin Source File

SOURCE=.\GPointerQueue.h
# End Source File
# Begin Source File

SOURCE=.\GPolynomial.cpp
# End Source File
# Begin Source File

SOURCE=.\GPolynomial.h
# End Source File
# Begin Source File

SOURCE=.\GQueue.cpp
# End Source File
# Begin Source File

SOURCE=.\GQueue.h
# End Source File
# Begin Source File

SOURCE=.\GRand.cpp
# End Source File
# Begin Source File

SOURCE=.\GRand.h
# End Source File
# Begin Source File

SOURCE=.\GRayTrace.cpp
# End Source File
# Begin Source File

SOURCE=.\GRayTrace.h
# End Source File
# Begin Source File

SOURCE=.\GRiddle.cpp
# End Source File
# Begin Source File

SOURCE=.\GRiddle.h
# End Source File
# Begin Source File

SOURCE=.\GSocket.cpp
# End Source File
# Begin Source File

SOURCE=.\GSocket.h
# End Source File
# Begin Source File

SOURCE=.\GSphereImage.cpp
# End Source File
# Begin Source File

SOURCE=.\GSphereImage.h
# End Source File
# Begin Source File

SOURCE=.\GSpinLock.cpp
# End Source File
# Begin Source File

SOURCE=.\GSpinLock.h
# End Source File
# Begin Source File

SOURCE=.\GStack.cpp
# End Source File
# Begin Source File

SOURCE=.\GStack.h
# End Source File
# Begin Source File

SOURCE=.\GStream.cpp
# End Source File
# Begin Source File

SOURCE=.\GStream.h
# End Source File
# Begin Source File

SOURCE=.\GString.cpp
# End Source File
# Begin Source File

SOURCE=.\GString.h
# End Source File
# Begin Source File

SOURCE=.\GThread.cpp
# End Source File
# Begin Source File

SOURCE=.\GThread.h
# End Source File
# Begin Source File

SOURCE=.\GTime.cpp
# End Source File
# Begin Source File

SOURCE=.\GTime.h
# End Source File
# Begin Source File

SOURCE=.\GTimer.cpp
# End Source File
# Begin Source File

SOURCE=.\GTimer.h
# End Source File
# Begin Source File

SOURCE=.\GTrie.cpp
# End Source File
# Begin Source File

SOURCE=.\GTrie.h
# End Source File
# Begin Source File

SOURCE=.\GTrigTable.cpp
# End Source File
# Begin Source File

SOURCE=.\GTrigTable.h
# End Source File
# Begin Source File

SOURCE=.\GVector.cpp
# End Source File
# Begin Source File

SOURCE=.\GVector.h
# End Source File
# Begin Source File

SOURCE=.\GVirtualMem.cpp
# End Source File
# Begin Source File

SOURCE=.\GWidgets.cpp
# End Source File
# Begin Source File

SOURCE=.\GWidgets.h
# End Source File
# Begin Source File

SOURCE=.\GWindows.cpp
# End Source File
# Begin Source File

SOURCE=.\GWindows.h
# End Source File
# Begin Source File

SOURCE=.\GXML.cpp
# End Source File
# Begin Source File

SOURCE=.\GXML.h
# End Source File
# Begin Source File

SOURCE=.\sha1.cpp
# End Source File
# Begin Source File

SOURCE=.\sha1.h
# End Source File
# Begin Source File

SOURCE=.\sha2.cpp
# End Source File
# Begin Source File

SOURCE=.\sha2.h
# End Source File
# Begin Source File

SOURCE=.\uitypes.h
# End Source File
# End Group
# End Target
# End Project
