################
# PATHS AND NAMES
################
TARGET_PATH = ../../../bin
TARGET_NAME_OPT = Gasp.exe
TARGET_NAME_DBG = GaspDbg.exe
OBJ_PATH = ../../../obj/obj
COMMAND_PATH = .
TEST_PATH = ../Test
UNAME = $(shell uname -s)
CFLAGS = $(shell sdl-config --cflags)
DBG_LIBRARIES = ../../../lib/GaspEngineDbg.a ../../../lib/GaspLibDbg.a ../../../lib/GaspSdlDbg.a ../../../lib/GClassesDbg.a
OPT_LIBRARIES = ../../../lib/GaspEngine.a ../../../lib/GaspLib.a ../../../lib/GaspSdl.a ../../../lib/GClasses.a
ifeq ($(UNAME),Darwin)
DBG_CFLAGS = $(CFLAGS) -g -D_DEBUG -DDARWIN -no-cpp-precomp
OPT_CFLAGS = $(CFLAGS) -O3 -DDARWIN -no-cpp-precomp
DBG_LFLAGS = $(DBG_LIBRARIES) $(shell sdl-config --libs) /System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib /sw/lib/libpng.3.1.2.5.dylib
OPT_LFLAGS = $(OPT_LIBRARIES) $(shell sdl-config --libs) /System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib /sw/lib/libpng.3.1.2.5.dylib
else
DBG_CFLAGS = $(CFLAGS) -g -D_DEBUG
OPT_CFLAGS = $(CFLAGS) -O3
DBG_LFLAGS = $(DBG_LIBRARIES) $(shell sdl-config --libs) /usr/lib/libGL.so.1 /usr/lib/libpng.so
OPT_LFLAGS = $(OPT_LIBRARIES) $(shell sdl-config --libs) /usr/lib/libGL.so.1 /usr/lib/libpng.so
endif

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

TEMP_LIST_7 = $(COMMAND_SOURCES:$(COMMAND_PATH)/%=$(OBJ_PATH)/%)
COMMAND_OBJECTS_OPT = $(TEMP_LIST_7:%.cpp=%.o)
COMMAND_OBJECTS_DBG = $(TEMP_LIST_7:%.cpp=%.dbg.o)

TEMP_LIST_8 = $(TEST_SOURCES:$(TEST_PATH)/%=$(OBJ_PATH)/%)
TEST_OBJECTS_OPT = $(TEMP_LIST_8:%.cpp=%.o)
TEST_OBJECTS_DBG = $(TEMP_LIST_8:%.cpp=%.dbg.o)

ALL_OBJECTS_OPT = $(COMMAND_OBJECTS_OPT) $(TEST_OBJECTS_OPT)
ALL_OBJECTS_DBG = $(COMMAND_OBJECTS_DBG) $(TEST_OBJECTS_DBG)
ALL_SOURCES = $(COMMAND_SOURCES) $(TEST_SOURCES)
ALL_HEADERS = $(COMMAND_HEADER) $(TEST_HEADER)

################
# Rules
################

dbg : skipsomelines $(TARGET_PATH)/$(TARGET_NAME_DBG) ../../../lib/GClassesDbg.a

opt : skipsomelines $(TARGET_PATH)/$(TARGET_NAME_OPT) ../../../lib/GClasses.a

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
	#

$(TARGET_PATH)/$(TARGET_NAME_OPT) : $(ALL_OBJECTS_OPT)
	g++ -lpthread -O3 -o $(TARGET_PATH)/$(TARGET_NAME_OPT) $(ALL_OBJECTS_OPT) $(OPT_LFLAGS)

$(TARGET_PATH)/$(TARGET_NAME_DBG) : $(ALL_OBJECTS_DBG)
	g++ -g -D_DEBUG -o $(TARGET_PATH)/$(TARGET_NAME_DBG) $(ALL_OBJECTS_DBG) $(DBG_LFLAGS)

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
	rm -f $(TARGET_PATH)/Gasp.exe
	rm -f $(TARGET_PATH)/GaspDbg.exe
	$(MAKE) -C ../Engine clean
	$(MAKE) -C ../MachineObjects clean
	$(MAKE) -C ../GaspSDL clean
