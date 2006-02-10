################
# PATHS AND NAMES
################
TARGET_PATH = ../../bin
TARGET_NAME_OPT = Isotope
TARGET_NAME_DBG = IsotopeDbg
OBJ_PATH = ../../obj/obj
ISOTOPE_PATH = .
UNAME = $(shell uname -s)
DBG_LIBRARIES = ../../lib/GaspEngineDbg.a ../../lib/GaspLibDbg.a ../../lib/GaspSdlDbg.a ../../lib/GClassesDbg.a
OPT_LIBRARIES = ../../lib/GaspEngine.a ../../lib/GaspLib.a ../../lib/GaspSdl.a ../../lib/GClasses.a
ifeq ($(UNAME),Darwin)
CFLAGS = -I/usr/local/include/SDL -D_THREAD_SAFE
DBG_CFLAGS = $(CFLAGS) -g -D_DEBUG -DDARWIN -DBIG_ENDIAN -no-cpp-precomp
OPT_CFLAGS = $(CFLAGS) -O3 -DDARWIN -DBIG_ENDIAN -no-cpp-precomp
DBG_LFLAGS = $(DBG_LIBRARIES) /usr/local/lib/libSDLmain.a -framework OpenGL -framework AGL -framework AudioToolbox -framework AudioUnit -framework IOKit -lstdc++ -framework Cocoa -framework Carbon -framework QuickTime /usr/local/lib/libpng.a /usr/local/lib/libSDL_mixer.a -lz /usr/local/lib/libvorbis.a /usr/local/lib/libvorbisfile.a /usr/local/lib/libogg.a /usr/local/lib/libSDL.a
OPT_LFLAGS = $(OPT_LIBRARIES) /usr/local/lib/libSDLmain.a -framework OpenGL -framework AGL -framework AudioToolbox -framework AudioUnit -framework IOKit -lstdc++ -framework Cocoa -framework Carbon -framework QuickTime /usr/local/lib/libpng.a /usr/local/lib/libSDL_mixer.a -lz /usr/local/lib/libvorbis.a /usr/local/lib/libvorbisfile.a /usr/local/lib/libogg.a /usr/local/lib/libSDL.a
else
CFLAGS = $(shell sdl-config --cflags)
DBG_CFLAGS = $(CFLAGS) -g -D_DEBUG
OPT_CFLAGS = $(CFLAGS) -O3
DBG_LFLAGS = $(DBG_LIBRARIES) -lSDL_mixer $(shell sdl-config --static-libs) /usr/lib/libGL.so.1 /usr/lib/libpng.so
OPT_LFLAGS = $(OPT_LIBRARIES) -lSDL_mixer $(shell sdl-config --static-libs) /usr/lib/libGL.so.1 /usr/lib/libpng.so
endif

################
# Isotope Sources
################

ISOTOPE_SOURCES =   $(ISOTOPE_PATH)/AutoUpdate.cpp\
                    $(ISOTOPE_PATH)/Controller.cpp\
                    $(ISOTOPE_PATH)/LPS.cpp\
                    $(ISOTOPE_PATH)/Main.cpp\
                    $(ISOTOPE_PATH)/MAnimation.cpp\
                    $(ISOTOPE_PATH)/MCollisionMap.cpp\
                    $(ISOTOPE_PATH)/MGameClient.cpp\
                    $(ISOTOPE_PATH)/MGameImage.cpp\
                    $(ISOTOPE_PATH)/MKeyPair.cpp\
                    $(ISOTOPE_PATH)/MObject.cpp\
                    $(ISOTOPE_PATH)/Model.cpp\
                    $(ISOTOPE_PATH)/MPuzSearchEngine.cpp\
                    $(ISOTOPE_PATH)/MRealm.cpp\
                    $(ISOTOPE_PATH)/MRealmServer.cpp\
                    $(ISOTOPE_PATH)/MScriptEngine.cpp\
                    $(ISOTOPE_PATH)/MSpot.cpp\
                    $(ISOTOPE_PATH)/MStatCollector.cpp\
                    $(ISOTOPE_PATH)/MStore.cpp\
                    $(ISOTOPE_PATH)/PuzzleGenerator.cpp\
                    $(ISOTOPE_PATH)/NRealmProtocol.cpp\
                    $(ISOTOPE_PATH)/VCharMake.cpp\
                    $(ISOTOPE_PATH)/VCharSelect.cpp\
                    $(ISOTOPE_PATH)/VEntropyCollector.cpp\
                    $(ISOTOPE_PATH)/VGame.cpp\
                    $(ISOTOPE_PATH)/View.cpp\
                    $(ISOTOPE_PATH)/VError.cpp\
                    $(ISOTOPE_PATH)/ViewPort.cpp\
                    $(ISOTOPE_PATH)/VLoading.cpp\
                    $(ISOTOPE_PATH)/VMainMenu.cpp\
                    $(ISOTOPE_PATH)/VPanel.cpp\
                    $(ISOTOPE_PATH)/VServer.cpp\
                    $(ISOTOPE_PATH)/VWave.cpp\

ISOTOPE_HEADER =    $(ISOTOPE_PATH)/AutoUpdate.h\
                    $(ISOTOPE_PATH)/Controller.h\
                    $(ISOTOPE_PATH)/LPS.h\
                    $(ISOTOPE_PATH)/Main.h\
                    $(ISOTOPE_PATH)/MAnimation.h\
                    $(ISOTOPE_PATH)/MCollisionMap.h\
                    $(ISOTOPE_PATH)/MGameClient.h\
                    $(ISOTOPE_PATH)/MGameImage.h\
                    $(ISOTOPE_PATH)/MKeyPair.h\
                    $(ISOTOPE_PATH)/MObject.h\
                    $(ISOTOPE_PATH)/Model.h\
                    $(ISOTOPE_PATH)/MPuzSearchEngine.h\
                    $(ISOTOPE_PATH)/MRealm.h\
                    $(ISOTOPE_PATH)/MRealmServer.h\
                    $(ISOTOPE_PATH)/MScriptEngine.h\
                    $(ISOTOPE_PATH)/MSpot.h\
                    $(ISOTOPE_PATH)/MStatCollector.h\
                    $(ISOTOPE_PATH)/MStore.h\
                    $(ISOTOPE_PATH)/PuzzleGenerator.h\
                    $(ISOTOPE_PATH)/NRealmProtocol.h\
                    $(ISOTOPE_PATH)/VCharMake.h\
                    $(ISOTOPE_PATH)/VCharSelect.h\
                    $(ISOTOPE_PATH)/VEntropyCollector.h\
                    $(ISOTOPE_PATH)/VGame.h\
                    $(ISOTOPE_PATH)/View.h\
                    $(ISOTOPE_PATH)/VError.h\
                    $(ISOTOPE_PATH)/ViewPort.h\
                    $(ISOTOPE_PATH)/VLoading.h\
                    $(ISOTOPE_PATH)/VPanel.h\
                    $(ISOTOPE_PATH)/VMainMenu.h\
                    $(ISOTOPE_PATH)/VServer.h\
                    $(ISOTOPE_PATH)/VWave.h\

################
# Lists
################

TEMP_LIST_8 = $(ISOTOPE_SOURCES:$(ISOTOPE_PATH)/%=$(OBJ_PATH)/%)
ISOTOPE_OBJECTS_OPT = $(TEMP_LIST_8:%.cpp=%.o)
ISOTOPE_OBJECTS_DBG = $(TEMP_LIST_8:%.cpp=%.dbg.o)

ALL_OBJECTS_OPT = $(ISOTOPE_OBJECTS_OPT)
ALL_OBJECTS_DBG = $(ISOTOPE_OBJECTS_DBG)
ALL_SOURCES = $(ISOTOPE_SOURCES)
ALL_HEADERS = $(ISOTOPE_HEADER)

################
# Rules
################

dbg : skipsomelines $(TARGET_PATH)/$(TARGET_NAME_DBG) $(DBG_LIBRARIES)

opt : skipsomelines $(TARGET_PATH)/$(TARGET_NAME_OPT) $(OPT_LIBRARIES)

skipsomelines :
	echo
	echo                    ################
	echo                        Isotope
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
	g++ -lpthread -O3 -o $(TARGET_PATH)/$(TARGET_NAME_OPT) $(ALL_OBJECTS_OPT) $(OPT_LFLAGS)

$(TARGET_PATH)/$(TARGET_NAME_DBG) : $(ALL_OBJECTS_DBG)
	g++ -g -D_DEBUG -o $(TARGET_PATH)/$(TARGET_NAME_DBG) $(ALL_OBJECTS_DBG) $(DBG_LFLAGS)

$(ISOTOPE_OBJECTS_OPT) : $(OBJ_PATH)/%.o : $(ISOTOPE_PATH)/%.cpp $(ALL_HEADERS)
	g++  $(OPT_CFLAGS) -c $< -o $@

$(ISOTOPE_OBJECTS_DBG) : $(OBJ_PATH)/%.dbg.o : $(ISOTOPE_PATH)/%.cpp $(ALL_HEADERS)
	g++  $(DBG_CFLAGS) -c $< -o $@

clean :
	rm -f $(TARGET_PATH)/$(TARGET_NAME_OPT)
	rm -f $(TARGET_PATH)/$(TARGET_NAME_DBG)
	rm -f $(ALL_OBJECTS_OPT)
	rm -f $(ALL_OBJECTS_DBG)
