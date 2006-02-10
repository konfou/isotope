################
# PATHS AND NAMES
################
TARGET_PATH = ../../../lib
TARGET_NAME_OPT = GaspLib.a
TARGET_NAME_DBG = GaspLibDbg.a
OBJ_PATH = ../../../obj/obj
MACHINE_PATH = .
UNAME = $(shell uname -s)
ifeq ($(UNAME),Darwin)
CFLAGS = -I/usr/local/include/SDL -D_THREAD_SAFE
DBG_CFLAGS = $(CFLAGS) -g -D_DEBUG -DDARWIN -DBIG_ENDIAN -no-cpp-precomp
OPT_CFLAGS = $(CFLAGS) -O3 -DDARWIN -DBIG_ENDIAN -no-cpp-precomp
else
CFLAGS = $(shell sdl-config --cflags)
DBG_CFLAGS = $(CFLAGS) -g -D_DEBUG
OPT_CFLAGS = $(CFLAGS) -O3
endif

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
# Lists
################

TEMP_LIST_5 = $(MACHINE_SOURCES:$(MACHINE_PATH)/%=$(OBJ_PATH)/%)
MACHINE_OBJECTS_OPT = $(TEMP_LIST_5:%.cpp=%.o)
MACHINE_OBJECTS_DBG = $(TEMP_LIST_5:%.cpp=%.dbg.o)

ALL_OBJECTS_OPT = $(MACHINE_OBJECTS_OPT)
ALL_OBJECTS_DBG = $(MACHINE_OBJECTS_DBG)
ALL_SOURCES = $(MACHINE_SOURCES)
ALL_HEADERS = $(MACHINE_HEADER)

################
# Rules
################

dbg : skipsomelines $(TARGET_PATH)/$(TARGET_NAME_DBG)

opt : skipsomelines $(TARGET_PATH)/$(TARGET_NAME_OPT)

skipsomelines :
	echo
	echo                    ################
	echo                        GaspLib
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
	ar -cr $(TARGET_PATH)/$(TARGET_NAME_OPT) $(ALL_OBJECTS_OPT)

$(TARGET_PATH)/$(TARGET_NAME_DBG) : $(ALL_OBJECTS_DBG)
	ar -cr $(TARGET_PATH)/$(TARGET_NAME_DBG) $(ALL_OBJECTS_DBG)

$(MACHINE_OBJECTS_OPT) : $(OBJ_PATH)/%.o : $(MACHINE_PATH)/%.cpp $(ALL_HEADERS)
	g++ $(OPT_CFLAGS) -c $< -o $@

$(MACHINE_OBJECTS_DBG) : $(OBJ_PATH)/%.dbg.o : $(MACHINE_PATH)/%.cpp $(ALL_HEADERS)
	g++ $(DBG_CFLAGS) -c $< -o $@

clean :
	rm -f $(ALL_OBJECTS_OPT)
	rm -f $(ALL_OBJECTS_DBG)
	rm -f $(TARGET_PATH)/GaspLib.a
	rm -f $(TARGET_PATH)/GaspLibDbg.a
