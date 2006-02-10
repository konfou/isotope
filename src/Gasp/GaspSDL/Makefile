################
# PATHS AND NAMES
################
TARGET_PATH = ../../../lib
TARGET_NAME_OPT = GaspSdl.a
TARGET_NAME_DBG = GaspSdlDbg.a
OBJ_PATH = ../../../obj/obj
GASHSDL_PATH = .
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
# GaspSDL Sources
################

GASHSDL_SOURCES =   $(GASHSDL_PATH)/GaspSDL.cpp\
                    $(GASHSDL_PATH)/Editor.cpp\
                    $(GASHSDL_PATH)/SdlFrame.cpp\
                    $(GASHSDL_PATH)/SdlWindow.cpp\
                    $(GASHSDL_PATH)/SdlScene.cpp\

GASHSDL_HEADER =    $(GASHSDL_PATH)/SdlFrame.h\
                    $(GASHSDL_PATH)/Editor.h\
                    $(GASHSDL_PATH)/SdlWindow.h\
                    $(GASHSDL_PATH)/SdlScene.h\

################
# Lists
################

TEMP_LIST_6 = $(GASHSDL_SOURCES:$(GASHSDL_PATH)/%=$(OBJ_PATH)/%)
GASHSDL_OBJECTS_OPT = $(TEMP_LIST_6:%.cpp=%.o)
GASHSDL_OBJECTS_DBG = $(TEMP_LIST_6:%.cpp=%.dbg.o)

ALL_OBJECTS_OPT = $(GASHSDL_OBJECTS_OPT)
ALL_OBJECTS_DBG = $(GASHSDL_OBJECTS_DBG)
ALL_SOURCES = $(GASHSDL_SOURCES)
ALL_HEADERS = $(GASHSDL_HEADER)

################
# Rules
################

dbg : skipsomelines $(TARGET_PATH)/$(TARGET_NAME_DBG)

opt : skipsomelines $(TARGET_PATH)/$(TARGET_NAME_OPT)

skipsomelines :
	echo
	echo                    ################
	echo                         GaspSDL
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

$(GASHSDL_OBJECTS_OPT) : $(OBJ_PATH)/%.o : $(GASHSDL_PATH)/%.cpp $(ALL_HEADERS)
	g++  $(OPT_CFLAGS) -c $< -o $@

$(GASHSDL_OBJECTS_DBG) : $(OBJ_PATH)/%.dbg.o : $(GASHSDL_PATH)/%.cpp $(ALL_HEADERS)
	g++  $(DBG_CFLAGS) -c $< -o $@

clean :
	rm -f $(ALL_OBJECTS_OPT)
	rm -f $(ALL_OBJECTS_DBG)
	rm -f $(TARGET_PATH)/GaspSdl.a
	rm -f $(TARGET_PATH)/GaspSdlDbg.a
