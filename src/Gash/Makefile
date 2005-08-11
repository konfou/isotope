################
# PATHS AND NAMES
################
TARGET_PATH = ../../bin
TARGET_NAME_OPT = gash.exe
TARGET_NAME_DBG = gashdbg.exe
OBJ_PATH = ../../obj/obj
GCLASSES_PATH = ../GClasses
ENGINE_PATH = Engine
CODEOBJ_PATH = CodeObjects
BUILTINS_PATH = BuiltIns
MACHINE_PATH = MachineObjects
GASHSDL_PATH = GashSDL
COMMAND_PATH = Command
TEST_PATH = Test
UNAME = $(shell uname -s)
CFLAGS = $(shell sdl-config --cflags)
ifeq ($(UNAME),Darwin)
DBG_CFLAGS = $(CFLAGS) -g -D_DEBUG -DDARWIN -no-cpp-precomp
OPT_CFLAGS = $(CFLAGS) -O3 -DDARWIN -no-cpp-precomp
#LFLAGS = $(shell sdl-config --libs) /usr/X11R6/lib/libGL.dylib
LFLAGS = $(shell sdl-config --libs) /System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib /sw/lib/libpng.3.1.2.5.dylib
else
DBG_CFLAGS = $(CFLAGS) -g -D_DEBUG
OPT_CFLAGS = $(CFLAGS) -O3
LFLAGS = $(shell sdl-config --libs) /usr/X11R6/lib/libGL.so /usr/lib/libpng.so
endif

################
# GClasses Sources
################

GCLASSES_SOURCES = $(GCLASSES_PATH)/GArff.cpp\
                   $(GCLASSES_PATH)/GArray.cpp\
                   $(GCLASSES_PATH)/GAVLTree.cpp\
                   $(GCLASSES_PATH)/GBezier.cpp\
                   $(GCLASSES_PATH)/GBigNumber.cpp\
                   $(GCLASSES_PATH)/GBillboardCamera.cpp\
                   $(GCLASSES_PATH)/GBitTable.cpp\
                   $(GCLASSES_PATH)/GCompress.cpp\
                   $(GCLASSES_PATH)/GCrypto.cpp\
                   $(GCLASSES_PATH)/GDataBase.cpp\
                   $(GCLASSES_PATH)/GDate.cpp\
                   $(GCLASSES_PATH)/GDiff.cpp\
                   $(GCLASSES_PATH)/GDirList.cpp\
                   $(GCLASSES_PATH)/GEZSocket.cpp\
                   $(GCLASSES_PATH)/GFile.cpp\
                   $(GCLASSES_PATH)/GFourier.cpp\
                   $(GCLASSES_PATH)/GGrammar.cpp\
                   $(GCLASSES_PATH)/GHardFont.cpp\
                   $(GCLASSES_PATH)/GHashTable.cpp\
                   $(GCLASSES_PATH)/GHeap.cpp\
                   $(GCLASSES_PATH)/GHttp.cpp\
                   $(GCLASSES_PATH)/GImage.cpp\
                   $(GCLASSES_PATH)/GKeyPair.cpp\
                   $(GCLASSES_PATH)/GLList.cpp\
                   $(GCLASSES_PATH)/GMath.cpp\
                   $(GCLASSES_PATH)/GMatrix.cpp\
                   $(GCLASSES_PATH)/GMipsEmulator.cpp\
                   $(GCLASSES_PATH)/GParseTree.cpp\
                   $(GCLASSES_PATH)/GPNG.cpp\
                   $(GCLASSES_PATH)/GPointerQueue.cpp\
                   $(GCLASSES_PATH)/GPolynomial.cpp\
                   $(GCLASSES_PATH)/GQueue.cpp\
                   $(GCLASSES_PATH)/GRand.cpp\
                   $(GCLASSES_PATH)/GRayTrace.cpp\
                   $(GCLASSES_PATH)/GRiddle.cpp\
                   $(GCLASSES_PATH)/GSocket.cpp\
                   $(GCLASSES_PATH)/GSphereImage.cpp\
                   $(GCLASSES_PATH)/GSpinLock.cpp\
                   $(GCLASSES_PATH)/GStack.cpp\
                   $(GCLASSES_PATH)/GStream.cpp\
                   $(GCLASSES_PATH)/GString.cpp\
                   $(GCLASSES_PATH)/GThread.cpp\
                   $(GCLASSES_PATH)/GTime.cpp\
                   $(GCLASSES_PATH)/GTrie.cpp\
                   $(GCLASSES_PATH)/GTrigTable.cpp\
                   $(GCLASSES_PATH)/GVirtualMem.cpp\
                   $(GCLASSES_PATH)/GWidgets.cpp\
                   $(GCLASSES_PATH)/GXML.cpp\
                   $(GCLASSES_PATH)/sha1.cpp\
                   $(GCLASSES_PATH)/sha2.cpp

GCLASSES_HEADER =  $(GCLASSES_PATH)/GArff.h\
                   $(GCLASSES_PATH)/GArray.h\
                   $(GCLASSES_PATH)/GAVLTree.h\
                   $(GCLASSES_PATH)/GBezier.h\
                   $(GCLASSES_PATH)/GBigNumber.h\
                   $(GCLASSES_PATH)/GBillboardCamera.h\
                   $(GCLASSES_PATH)/GBitTable.h\
                   $(GCLASSES_PATH)/GBuffer.h\
                   $(GCLASSES_PATH)/GCompress.h\
                   $(GCLASSES_PATH)/GCrypto.h\
                   $(GCLASSES_PATH)/GDataBase.h\
                   $(GCLASSES_PATH)/GEZSocket.h\
                   $(GCLASSES_PATH)/GFile.h\
                   $(GCLASSES_PATH)/GFlipTable.h\
                   $(GCLASSES_PATH)/GFourier.h\
                   $(GCLASSES_PATH)/GGrammar.h\
                   $(GCLASSES_PATH)/GHardFont.h\
                   $(GCLASSES_PATH)/GHashTable.h\
                   $(GCLASSES_PATH)/GHeap.h\
                   $(GCLASSES_PATH)/GHttp.h\
                   $(GCLASSES_PATH)/GImage.h\
                   $(GCLASSES_PATH)/GKeyPair.h\
                   $(GCLASSES_PATH)/GLList.h\
                   $(GCLASSES_PATH)/GMacros.h\
                   $(GCLASSES_PATH)/GMemChunk.h\
                   $(GCLASSES_PATH)/GParseTree.h\
                   $(GCLASSES_PATH)/GPNG.h\
                   $(GCLASSES_PATH)/GPointerQueue.h\
                   $(GCLASSES_PATH)/GPolynomial.h\
                   $(GCLASSES_PATH)/GQueue.h\
                   $(GCLASSES_PATH)/GRand.h\
                   $(GCLASSES_PATH)/GRayTrace.h\
                   $(GCLASSES_PATH)/GRiddle.h\
                   $(GCLASSES_PATH)/GSocket.h\
                   $(GCLASSES_PATH)/GSphereImage.h\
                   $(GCLASSES_PATH)/GSpinLock.h\
                   $(GCLASSES_PATH)/GString.h\
                   $(GCLASSES_PATH)/GThread.h\
                   $(GCLASSES_PATH)/GTime.h\
                   $(GCLASSES_PATH)/GVector.h\
                   $(GCLASSES_PATH)/GWidgets.h\
                   $(GCLASSES_PATH)/GXML.h\
                   $(GCLASSES_PATH)/sha1.h\
                   $(GCLASSES_PATH)/sha2.h\
                   $(GCLASSES_PATH)/uitypes.h

################
# Engine Sources
################

ENGINE_SOURCES =   $(ENGINE_PATH)/ClassicSyntax.cpp\
                   $(ENGINE_PATH)/Commands.cpp\
                   $(ENGINE_PATH)/Disassembler.cpp\
                   $(ENGINE_PATH)/EClass.cpp\
                   $(ENGINE_PATH)/EInstrArray.cpp\
                   $(ENGINE_PATH)/EInterface.cpp\
                   $(ENGINE_PATH)/EMethod.cpp\
                   $(ENGINE_PATH)/EType.cpp\
                   $(ENGINE_PATH)/Engine.cpp\
                   $(ENGINE_PATH)/Error.cpp\
                   $(ENGINE_PATH)/EvalExprResult.cpp\
                   $(ENGINE_PATH)/GCompiler.cpp\
                   $(ENGINE_PATH)/Gvm.cpp\
                   $(ENGINE_PATH)/GVMStack.cpp\
                   $(ENGINE_PATH)/InstrSet.cpp\
                   $(ENGINE_PATH)/InstrTable.cpp\
                   $(ENGINE_PATH)/Library.cpp\
                   $(ENGINE_PATH)/Optimizer.cpp\
                   $(ENGINE_PATH)/TagNames.cpp\
                   $(ENGINE_PATH)/VarHolder.cpp\

ENGINE_HEADER =    $(ENGINE_PATH)/ClassicSyntax.h\
                   $(ENGINE_PATH)/Disassembler.h\
                   $(ENGINE_PATH)/EClass.h\
                   $(ENGINE_PATH)/EInstrArray.h\
                   $(ENGINE_PATH)/EInterface.h\
                   $(ENGINE_PATH)/EMethod.h\
                   $(ENGINE_PATH)/Error.h\
                   $(ENGINE_PATH)/ErrorMessages.h\
                   $(ENGINE_PATH)/EvalExprResult.h\
                   $(ENGINE_PATH)/GCompiler.h\
                   $(ENGINE_PATH)/GVMStack.h\
                   $(ENGINE_PATH)/InstrSet.h\
                   $(ENGINE_PATH)/InstrTable.h\
                   $(ENGINE_PATH)/Optimizer.h\
                   $(ENGINE_PATH)/TagNames.h\

################
# CodeObject Sources
################

CODEOBJ_SOURCES =  $(CODEOBJ_PATH)/Block.cpp\
                   $(CODEOBJ_PATH)/Call.cpp\
                   $(CODEOBJ_PATH)/Class.cpp\
                   $(CODEOBJ_PATH)/CodeObject.cpp\
                   $(CODEOBJ_PATH)/Constant.cpp\
                   $(CODEOBJ_PATH)/Expression.cpp\
                   $(CODEOBJ_PATH)/File.cpp\
                   $(CODEOBJ_PATH)/FileSet.cpp\
                   $(CODEOBJ_PATH)/InstrArray.cpp\
                   $(CODEOBJ_PATH)/Instruction.cpp\
                   $(CODEOBJ_PATH)/Interface.cpp\
                   $(CODEOBJ_PATH)/Method.cpp\
                   $(CODEOBJ_PATH)/Operator.cpp\
                   $(CODEOBJ_PATH)/Project.cpp\
                   $(CODEOBJ_PATH)/Scope.cpp\
                   $(CODEOBJ_PATH)/Type.cpp\
                   $(CODEOBJ_PATH)/Variable.cpp\
                   $(CODEOBJ_PATH)/VarRef.cpp\

CODEOBJ_HEADER =   $(CODEOBJ_PATH)/Block.h\
                   $(CODEOBJ_PATH)/Call.h\
                   $(CODEOBJ_PATH)/Class.h\
                   $(CODEOBJ_PATH)/CodeObject.h\
                   $(CODEOBJ_PATH)/Constant.h\
                   $(CODEOBJ_PATH)/Expression.h\
                   $(CODEOBJ_PATH)/File.h\
                   $(CODEOBJ_PATH)/FileSet.h\
                   $(CODEOBJ_PATH)/InstrArray.h\
                   $(CODEOBJ_PATH)/Instruction.h\
                   $(CODEOBJ_PATH)/Interface.h\
                   $(CODEOBJ_PATH)/Method.h\
                   $(CODEOBJ_PATH)/Operator.h\
                   $(CODEOBJ_PATH)/Project.h\
                   $(CODEOBJ_PATH)/Scope.h\
                   $(CODEOBJ_PATH)/Type.h\
                   $(CODEOBJ_PATH)/Variable.h\
                   $(CODEOBJ_PATH)/VarRef.h\

################
# BuiltIns Sources
################

BUILTINS_SOURCES =  $(BUILTINS_PATH)/BuiltIns.cpp\
                    $(BUILTINS_PATH)/GashFloat.cpp\
                    $(BUILTINS_PATH)/GashStream.cpp\
                    $(BUILTINS_PATH)/GashString.cpp\
                    $(BUILTINS_PATH)/GObject.cpp\

BUILTINS_HEADER =   $(BUILTINS_PATH)/GashFloat.h\
                    $(BUILTINS_PATH)/GashStream.h\
                    $(BUILTINS_PATH)/GashString.h\
                    $(BUILTINS_PATH)/GObject.h\

################
# MachineObjects Sources
################

MACHINE_SOURCES =   $(MACHINE_PATH)/MachineObjects.cpp\
                    $(MACHINE_PATH)/MArray.cpp\
                    $(MACHINE_PATH)/MBigInt.cpp\
                    $(MACHINE_PATH)/MConsole.cpp\
                    $(MACHINE_PATH)/MEngine.cpp\
                    $(MACHINE_PATH)/MImage.cpp\
                    $(MACHINE_PATH)/MLibrary.cpp\

MACHINE_HEADER =    $(MACHINE_PATH)/MachineObjects.h\
                    $(MACHINE_PATH)/MArray.h\
                    $(MACHINE_PATH)/MBigInt.h\
                    $(MACHINE_PATH)/MConsole.h\
                    $(MACHINE_PATH)/MEngine.h\
                    $(MACHINE_PATH)/MImage.h\
                    $(MACHINE_PATH)/MLibrary.h\

################
# GashSDL Sources
################

GASHSDL_SOURCES =   $(GASHSDL_PATH)/GashSDL.cpp\
                    $(GASHSDL_PATH)/Editor.cpp\
                    $(GASHSDL_PATH)/SdlFrame.cpp\
                    $(GASHSDL_PATH)/SdlWindow.cpp\
                    $(GASHSDL_PATH)/SdlScene.cpp\

GASHSDL_HEADER =    $(GASHSDL_PATH)/SdlFrame.h\
                    $(GASHSDL_PATH)/Editor.h\
                    $(GASHSDL_PATH)/SdlWindow.h\
                    $(GASHSDL_PATH)/SdlScene.h\

################
# Command Sources
################

COMMAND_SOURCES =   $(COMMAND_PATH)/Command.cpp\

COMMAND_HEADER =

################
# Test Sources
################

TEST_SOURCES =      $(TEST_PATH)/EngineTests.cpp\
                    $(TEST_PATH)/ClassTests.cpp\
                    $(TEST_PATH)/Class/TestAvlTree.cpp\
                    $(TEST_PATH)/Class/TestBigNumber.cpp\
                    $(TEST_PATH)/Class/TestDataBase.cpp\
                    $(TEST_PATH)/Class/TestHashTable.cpp\
                    $(TEST_PATH)/Class/TestMatrix.cpp\
                    $(TEST_PATH)/Class/TestPrecalcTrigTable.cpp\
                    $(TEST_PATH)/Class/TestSpinLock.cpp\

TEST_HEADER =       $(TEST_PATH)/Test.h\
                    $(TEST_PATH)/EngineTests.h\
                    $(TEST_PATH)/ClassTests.h\
                    $(TEST_PATH)/Class/TestAvlTree.h\
                    $(TEST_PATH)/Class/TestBigNumber.h\
                    $(TEST_PATH)/Class/TestDataBase.h\
                    $(TEST_PATH)/Class/TestHashTable.h\
                    $(TEST_PATH)/Class/TestMatrix.h\
                    $(TEST_PATH)/Class/TestPrecalcTrigTable.h\
                    $(TEST_PATH)/Class/TestSpinLock.h\

################
# Lists
################

TEMP_LIST_1 = $(GCLASSES_SOURCES:$(GCLASSES_PATH)/%=$(OBJ_PATH)/%)
GCLASSES_OBJECTS_OPT = $(TEMP_LIST_1:%.cpp=%.o)
GCLASSES_OBJECTS_DBG = $(TEMP_LIST_1:%.cpp=%.dbg.o)

TEMP_LIST_2 = $(ENGINE_SOURCES:$(ENGINE_PATH)/%=$(OBJ_PATH)/%)
ENGINE_OBJECTS_OPT = $(TEMP_LIST_2:%.cpp=%.o)
ENGINE_OBJECTS_DBG = $(TEMP_LIST_2:%.cpp=%.dbg.o)

TEMP_LIST_3 = $(CODEOBJ_SOURCES:$(CODEOBJ_PATH)/%=$(OBJ_PATH)/%)
CODEOBJ_OBJECTS_OPT = $(TEMP_LIST_3:%.cpp=%.o)
CODEOBJ_OBJECTS_DBG = $(TEMP_LIST_3:%.cpp=%.dbg.o)

TEMP_LIST_4 = $(BUILTINS_SOURCES:$(BUILTINS_PATH)/%=$(OBJ_PATH)/%)
BUILTINS_OBJECTS_OPT = $(TEMP_LIST_4:%.cpp=%.o)
BUILTINS_OBJECTS_DBG = $(TEMP_LIST_4:%.cpp=%.dbg.o)

TEMP_LIST_5 = $(MACHINE_SOURCES:$(MACHINE_PATH)/%=$(OBJ_PATH)/%)
MACHINE_OBJECTS_OPT = $(TEMP_LIST_5:%.cpp=%.o)
MACHINE_OBJECTS_DBG = $(TEMP_LIST_5:%.cpp=%.dbg.o)

TEMP_LIST_6 = $(GASHSDL_SOURCES:$(GASHSDL_PATH)/%=$(OBJ_PATH)/%)
GASHSDL_OBJECTS_OPT = $(TEMP_LIST_6:%.cpp=%.o)
GASHSDL_OBJECTS_DBG = $(TEMP_LIST_6:%.cpp=%.dbg.o)

TEMP_LIST_7 = $(COMMAND_SOURCES:$(COMMAND_PATH)/%=$(OBJ_PATH)/%)
COMMAND_OBJECTS_OPT = $(TEMP_LIST_7:%.cpp=%.o)
COMMAND_OBJECTS_DBG = $(TEMP_LIST_7:%.cpp=%.dbg.o)

TEMP_LIST_8 = $(TEST_SOURCES:$(TEST_PATH)/%=$(OBJ_PATH)/%)
TEST_OBJECTS_OPT = $(TEMP_LIST_8:%.cpp=%.o)
TEST_OBJECTS_DBG = $(TEMP_LIST_8:%.cpp=%.dbg.o)

ALL_OBJECTS_OPT = $(GCLASSES_OBJECTS_OPT) $(ENGINE_OBJECTS_OPT) $(CODEOBJ_OBJECTS_OPT) $(BUILTINS_OBJECTS_OPT) $(MACHINE_OBJECTS_OPT) $(GASHSDL_OBJECTS_OPT) $(COMMAND_OBJECTS_OPT) $(TEST_OBJECTS_OPT)
ALL_OBJECTS_DBG = $(GCLASSES_OBJECTS_DBG) $(ENGINE_OBJECTS_DBG) $(CODEOBJ_OBJECTS_DBG) $(BUILTINS_OBJECTS_DBG) $(MACHINE_OBJECTS_DBG) $(GASHSDL_OBJECTS_DBG) $(COMMAND_OBJECTS_DBG) $(TEST_OBJECTS_DBG)
ALL_SOURCES = $(GCLASSES_SOURCES) $(ENGINE_SOURCES) $(CODEOBJ_SOURCES) $(BUILTINS_SOURCES) $(MACHINE_SOURCES) $(GASHSDL_SOURCES) $(COMMAND_SOURCES) $(TEST_SOURCES)
ALL_HEADERS = $(GCLASSES_HEADER) $(ENGINE_HEADERS) $(CODEOBJ_HEADERS) $(BUILTINS_HEADER) $(MACHINE_HEADER) $(GASHSDL_HEADER) $(COMMAND_HEADER) $(TEST_HEADER)

################
# Rules
################

dbg : skipsomelines $(TARGET_PATH)/$(TARGET_NAME_DBG)

opt : skipsomelines $(TARGET_PATH)/$(TARGET_NAME_OPT)

skipsomelines :
	echo
	echo
	echo
	echo
	echo
	echo
	echo
	echo
	echo
	
usage:
	#
	# Usage:
	#  make usage   (to see this info)
	#  make clean   (to delete all the .o files)
	#  make dbg     (to build a debug version)
	#  make opt     (to build an optimized version)
	#  make echo    (to list all the sources)
	#

$(TARGET_PATH)/$(TARGET_NAME_OPT) : $(ALL_OBJECTS_OPT)
	g++ -lpthread -O3 -o $(TARGET_PATH)/$(TARGET_NAME_OPT) $(ALL_OBJECTS_OPT) $(LFLAGS)

$(TARGET_PATH)/$(TARGET_NAME_DBG) : $(ALL_OBJECTS_DBG)
	g++ -g -D_DEBUG -o $(TARGET_PATH)/$(TARGET_NAME_DBG) $(ALL_OBJECTS_DBG) $(LFLAGS)

$(GCLASSES_OBJECTS_OPT) : $(OBJ_PATH)/%.o : $(GCLASSES_PATH)/%.cpp $(GCLASSES_HEADER)
	g++ $(OPT_CFLAGS) -c $< -o $@

$(GCLASSES_OBJECTS_DBG) : $(OBJ_PATH)/%.dbg.o : $(GCLASSES_PATH)/%.cpp $(GCLASSES_HEADER)
	g++ $(DBG_CFLAGS) -c $< -o $@

$(ENGINE_OBJECTS_OPT) : $(OBJ_PATH)/%.o : $(ENGINE_PATH)/%.cpp $(ALL_HEADERS)
	g++ $(OPT_CFLAGS) -c $< -o $@

$(ENGINE_OBJECTS_DBG) : $(OBJ_PATH)/%.dbg.o : $(ENGINE_PATH)/%.cpp $(ALL_HEADERS)
	g++ $(DBG_CFLAGS) -c $< -o $@

$(CODEOBJ_OBJECTS_OPT) : $(OBJ_PATH)/%.o : $(CODEOBJ_PATH)/%.cpp $(ALL_HEADERS)
	g++ $(OPT_CFLAGS) -c $< -o $@

$(CODEOBJ_OBJECTS_DBG) : $(OBJ_PATH)/%.dbg.o : $(CODEOBJ_PATH)/%.cpp $(ALL_HEADERS)
	g++ $(DBG_CFLAGS) -c $< -o $@

$(BUILTINS_OBJECTS_OPT) : $(OBJ_PATH)/%.o : $(BUILTINS_PATH)/%.cpp $(ALL_HEADERS)
	g++ $(OPT_CFLAGS) -c $< -o $@

$(BUILTINS_OBJECTS_DBG) : $(OBJ_PATH)/%.dbg.o : $(BUILTINS_PATH)/%.cpp $(ALL_HEADERS)
	g++ $(DBG_CFLAGS) -c $< -o $@

$(MACHINE_OBJECTS_OPT) : $(OBJ_PATH)/%.o : $(MACHINE_PATH)/%.cpp $(ALL_HEADERS)
	g++ $(OPT_CFLAGS) -c $< -o $@

$(MACHINE_OBJECTS_DBG) : $(OBJ_PATH)/%.dbg.o : $(MACHINE_PATH)/%.cpp $(ALL_HEADERS)
	g++ $(DBG_CFLAGS) -c $< -o $@

$(GASHSDL_OBJECTS_OPT) : $(OBJ_PATH)/%.o : $(GASHSDL_PATH)/%.cpp $(ALL_HEADERS)
	g++  $(OPT_CFLAGS) -c $< -o $@

$(GASHSDL_OBJECTS_DBG) : $(OBJ_PATH)/%.dbg.o : $(GASHSDL_PATH)/%.cpp $(ALL_HEADERS)
	g++  $(DBG_CFLAGS) -c $< -o $@

$(COMMAND_OBJECTS_OPT) : $(OBJ_PATH)/%.o : $(COMMAND_PATH)/%.cpp $(ALL_HEADERS)
	g++  $(OPT_CFLAGS) -c $< -o $@

$(COMMAND_OBJECTS_DBG) : $(OBJ_PATH)/%.dbg.o : $(COMMAND_PATH)/%.cpp $(ALL_HEADERS)
	g++  $(DBG_CFLAGS) -c $< -o $@

$(TEST_OBJECTS_OPT) : $(OBJ_PATH)/%.o : $(TEST_PATH)/%.cpp $(ALL_HEADERS)
	g++  $(OPT_CFLAGS) -c $< -o $@

$(TEST_OBJECTS_DBG) : $(OBJ_PATH)/%.dbg.o : $(TEST_PATH)/%.cpp $(ALL_HEADERS)
	g++  $(DBG_CFLAGS) -c $< -o $@

clean :
	rm -f $(ALL_OBJECTS_OPT)
	rm -f $(ALL_OBJECTS_DBG)

echo :
	echo ### SOURCES ###
	echo $(ALL_SOURCES)
	echo ### HEADERS ###
	echo $(ALL_HEADERS)
	echo ### OBJECTS ###
	echo $(ALL_OBJECTS_DBG)