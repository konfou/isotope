################
# PATHS AND NAMES
################
TARGET_PATH = ../../../lib
TARGET_NAME_OPT = GashSdl.a
TARGET_NAME_DBG = GashSdlDbg.a
OBJ_PATH = ../../../obj/obj
GASHSDL_PATH = .
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
	echo                         GashSDL
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

$(GASHSDL_OBJECTS_OPT) : $(OBJ_PATH)/%.o : $(GASHSDL_PATH)/%.cpp $(ALL_HEADERS)
	g++  $(OPT_CFLAGS) -c $< -o $@

$(GASHSDL_OBJECTS_DBG) : $(OBJ_PATH)/%.dbg.o : $(GASHSDL_PATH)/%.cpp $(ALL_HEADERS)
	g++  $(DBG_CFLAGS) -c $< -o $@

clean :
	rm -f $(ALL_OBJECTS_OPT)
	rm -f $(ALL_OBJECTS_DBG)
	rm -f $(TARGET_PATH)/GashSdl.a
	rm -f $(TARGET_PATH)/GashSdlDbg.a
