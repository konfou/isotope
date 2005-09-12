################
# PATHS AND NAMES
################
TARGET_PATH = ../../../lib
TARGET_NAME_OPT = GashEngine.a
TARGET_NAME_DBG = GashEngineDbg.a
OBJ_PATH = ../../../obj/obj
ENGINE_PATH = .
CODEOBJ_PATH = ../CodeObjects
BUILTINS_PATH = ../BuiltIns
UNAME = $(shell uname -s)
CFLAGS = $(shell sdl-config --cflags)
ifeq ($(UNAME),Darwin)
DBG_CFLAGS = $(CFLAGS) -g -D_DEBUG -DDARWIN -no-cpp-precomp
OPT_CFLAGS = $(CFLAGS) -O3 -DDARWIN -no-cpp-precomp
else
DBG_CFLAGS = $(CFLAGS) -g -D_DEBUG
OPT_CFLAGS = $(CFLAGS) -O3
endif

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
# Lists
################

TEMP_LIST_2 = $(ENGINE_SOURCES:$(ENGINE_PATH)/%=$(OBJ_PATH)/%)
ENGINE_OBJECTS_OPT = $(TEMP_LIST_2:%.cpp=%.o)
ENGINE_OBJECTS_DBG = $(TEMP_LIST_2:%.cpp=%.dbg.o)

TEMP_LIST_3 = $(CODEOBJ_SOURCES:$(CODEOBJ_PATH)/%=$(OBJ_PATH)/%)
CODEOBJ_OBJECTS_OPT = $(TEMP_LIST_3:%.cpp=%.o)
CODEOBJ_OBJECTS_DBG = $(TEMP_LIST_3:%.cpp=%.dbg.o)

TEMP_LIST_4 = $(BUILTINS_SOURCES:$(BUILTINS_PATH)/%=$(OBJ_PATH)/%)
BUILTINS_OBJECTS_OPT = $(TEMP_LIST_4:%.cpp=%.o)
BUILTINS_OBJECTS_DBG = $(TEMP_LIST_4:%.cpp=%.dbg.o)

ALL_OBJECTS_OPT = $(ENGINE_OBJECTS_OPT) $(CODEOBJ_OBJECTS_OPT) $(BUILTINS_OBJECTS_OPT)
ALL_OBJECTS_DBG = $(ENGINE_OBJECTS_DBG) $(CODEOBJ_OBJECTS_DBG) $(BUILTINS_OBJECTS_DBG)
ALL_SOURCES = $(ENGINE_SOURCES) $(CODEOBJ_SOURCES) $(BUILTINS_SOURCES)
ALL_HEADERS = $(ENGINE_HEADERS) $(CODEOBJ_HEADERS) $(BUILTINS_HEADER)

################
# Rules
################

dbg : skipsomelines $(TARGET_PATH)/$(TARGET_NAME_DBG)

opt : skipsomelines $(TARGET_PATH)/$(TARGET_NAME_OPT)

skipsomelines :
	echo
	echo                    ################
	echo                       GashEngine
	echo                    ################
	echo
	
usage:
	#
	# Usage:
	#  make usage   (to see this info)
	#  make clean   (to delete all the .o files)
	#  make dbg     (to build a debug version)
	#  make opt     (to build an optimized version)
	#

$(TARGET_PATH)/$(TARGET_NAME_OPT) : $(ALL_OBJECTS_OPT)
	ar -cq $(TARGET_PATH)/$(TARGET_NAME_OPT) $(ALL_OBJECTS_OPT)

$(TARGET_PATH)/$(TARGET_NAME_DBG) : $(ALL_OBJECTS_DBG)
	ar -cq $(TARGET_PATH)/$(TARGET_NAME_DBG) $(ALL_OBJECTS_DBG)

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

clean :
	rm -f $(ALL_OBJECTS_OPT)
	rm -f $(ALL_OBJECTS_DBG)
	rm -f $(TARGET_PATH)/GashEngine.a
	rm -f $(TARGET_PATH)/GashEngineDbg.a
